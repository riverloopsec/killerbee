"""
CC2530/CC2531 injection support is contributed by virtualabs.

This driver communicates with a CC2531 compatible USB dongle
flashed with 'Bumblebee' firmware. It is then able to communicate
with this dongle through USB (bulk data transfer).

Bumblebee firmware provides 2.4GHz sniffing and injection capabilities,
with RSSI/LQI. Bad FCS sniffing and jamming are not currently supported.

Notes on Bumblebee firmware 2.1 and later:
- Bumblebee fw 2.1r1 (kb-usb.c) sends every EP2 IN packet as 64 bytes:
  [chunk_len][chunk_len bytes of protocol stream][padding]. The chunk
  header and padding are now stripped before protocol parsing; the
  original driver parsed them as protocol data, so ACKs and frames were
  never matched.
- The firmware hangs (then the ~1 s watchdog resets the chip, seen by the
  host as EPIPE/ENODEV and a USB re-enumeration) if it queues a new message
  before the host has collected the previous multi-packet one. IN reads
  are now kept pending at all times (queued async transfers, or a
  blocking-read thread).
- The Contiki CC253x USB stack never resets its data toggles, so a host
  SET_CONFIGURATION on an already-configured dongle drops the next packet
  about half the time. set_configuration() is skipped when the kernel already
  selected configuration 1, and CMD_INIT is retried.
- Missing INIT/SET_CHANNEL/SNIFF_ON/SNIFF_OFF ACKs raise KBInterfaceError
  instead of being ignored.

Firmware with the fixes proposed to virtualabs/cc2531-killerbee-fw fixes the
IN-queue race and the data toggles in the firmware itself; the measures above
stay for 2.1r1 dongles. It also reports the radio transmit() result in
the SEND_PKT ACK and exposes the chip IEEE address as the USB serial number.
send_packet() returns False when that result is not RADIO_TX_OK; with stock
firmware (empty ACK) it still returns True on any ACK. The raw result is in
last_tx_status (None if the firmware did not report one).
"""

from __future__ import print_function # type: ignore
import sys # type: ignore
import struct # type: ignore
import time # type: ignore
import errno # type: ignore
import threading # type: ignore
from collections import deque # type: ignore
from datetime import datetime # type: ignore
from .kbutils import KBCapabilities, KBInterfaceError, makeFCS, bytearray_to_bytes # type: ignore

import ctypes # type: ignore
import usb.core # type: ignore
import usb.util # type: ignore

class _AsyncBulkIn(object):
    """
    Keeps NQUEUE libusb bulk-IN transfers queued on one endpoint so the host
    controller collects every packet even while Python is busy (the firmware
    corrupts its IN queue if a packet waits). Uses pyusb's libusb1 backend handle.
    """
    NQUEUE = 16
    _STATUS = {1: 'error', 2: 'timed out', 4: 'stall (EPIPE)', 5: 'no device', 6: 'overflow (babble)'}

    def __init__(self, dev, ep, size, on_data, on_error):
        from usb.backend import libusb1 as lu
        backend = dev._ctx.backend
        if not isinstance(backend, lu._LibUSB):
            raise TypeError("not the libusb1 backend")
        dev._ctx.managed_open()
        self.lib, self.ctx, self.handle = backend.lib, backend.ctx, dev._ctx.handle.handle
        self.lib.libusb_cancel_transfer.argtypes = [lu._libusb_transfer_p]
        self.lib.libusb_handle_events_timeout_completed.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
        self.on_data, self.on_error = on_data, on_error
        self.stopping = False
        self.pending = set()
        self._cb = lu._libusb_transfer_cb_fn_p(self._callback)
        self._bufs = []
        self._xfers = []
        for i in range(self.NQUEUE):
            t = self.lib.libusb_alloc_transfer(0)
            buf = ctypes.create_string_buffer(size)
            c = t.contents
            c.dev_handle = self.handle
            c.flags = 0
            c.endpoint = ep
            c.type = 2                      # LIBUSB_TRANSFER_TYPE_BULK
            c.timeout = 0                   # no timeout: always pending
            c.length = size
            c.callback = self._cb
            c.buffer = ctypes.cast(buf, ctypes.c_void_p)
            c.num_iso_packets = 0
            self._bufs.append(buf)
            self._xfers.append(t)
        for t in self._xfers:
            if self.lib.libusb_submit_transfer(t) != 0:
                self.stop()
                raise usb.core.USBError("libusb_submit_transfer failed")
            self.pending.add(ctypes.addressof(t.contents))
        self._thread = threading.Thread(target=self._events, name="bumblebee-usb-events", daemon=True)
        self._thread.start()

    def _callback(self, tp):
        c = tp.contents
        key = ctypes.addressof(c)
        if c.status == 0:
            if c.actual_length:
                self.on_data(ctypes.string_at(c.buffer, c.actual_length))
            if not self.stopping and self.lib.libusb_submit_transfer(tp) == 0:
                return
        elif c.status != 3 and not self.stopping:   # 3 = cancelled
            self.on_error(usb.core.USBError(self._STATUS.get(c.status, 'status %d' % c.status)))
            self.stopping = True
        self.pending.discard(key)

    def _events(self):
        tv = (ctypes.c_long * 2)(0, 100000)
        while self.pending:
            self.lib.libusb_handle_events_timeout_completed(self.ctx, ctypes.byref(tv), None)
            if self.stopping:
                for t in self._xfers:
                    if ctypes.addressof(t.contents) in self.pending:
                        self.lib.libusb_cancel_transfer(t)

    def stop(self):
        self.stopping = True
        if getattr(self, '_thread', None) is not None and self._thread is not threading.current_thread():
            self._thread.join(2.0)
        if not self.pending:
            for t in self._xfers:
                self.lib.libusb_free_transfer(t)
            self._xfers = []

class CommProtocolPacket(object):
    """
    USB communication protocol packet, used to communicate with
    our CC2531 dongle.
    """

    def __init__(self, command, data):
        self.__command = command
        self.__data = data

    def get_command(self):
        """
        Retrieve packet's command.
        """
        return self.__command

    def get_data(self):
        """
        Retrieve packet's data
        """
        return self.__data

class Bumblebee(object):
    """
    Bumblebee driver class.
    """

    # USB Endpoints
    EP_OUT = 0x03
    EP_IN = 0x82
    EP_SIZE = 64

    # Communication protocol commands
    CMD_INIT = 0x00
    CMD_INIT_ACK = 0x01
    CMD_SET_CHANNEL = 0x02
    CMD_SET_CHANNEL_ACK = 0x03
    CMD_SEND_PKT = 0x04
    CMD_SEND_PKT_ACK = 0x05
    CMD_SNIFF_ON = 0x06
    CMD_SNIFF_ON_ACK = 0x07
    CMD_SNIFF_OFF = 0x08
    CMD_SNIFF_OFF_ACK = 0x09
    CMD_GOT_PKT = 0x0A

    # SEND_PKT ACK payload (fixed firmware): Contiki radio transmit() result
    TX_OK = 0
    TX_ERR = 1
    TX_COLLISION = 2
    TX_STATUS_NAMES = {0: 'ok', 1: 'error', 2: 'collision (CCA busy or RX in progress)', 3: 'no ack'}


    def __init__(self, dev, bus):
        """
        Initialize device and capabilities.
        """
        if not isinstance(dev, usb.core.Device):
            raise KBInterfaceError("Bumblebee needs a usb.core.Device, got %r" % (dev,))

        self.dev = dev
        self.last_tx_status = None
        self._last_ack = None
        self.rx_buffer = bytes()
        self._channel = None
        self.__stream_open = False
        self.timeout = 2.0
        self.capabilities = KBCapabilities()
        self.__set_capabilities()
        self._frames = deque()
        self._rx_lock = threading.Condition()
        self._rx_error = None
        self._rx_stop = threading.Event()

        # Set configuration only if the kernel has not: a SET_CONFIGURATION resets
        # the host's data toggles but the firmware never resets its own, which
        # silently drops the next IN or OUT packet about half the time.
        try:
            cfg = self.dev.get_active_configuration()
        except usb.core.USBError:
            cfg = None
        if cfg is None or cfg.bConfigurationValue != 1:
            self.dev.set_configuration()
        usb.util.claim_interface(self.dev, 0)

        self.name = usb.util.get_string(self.dev, self.dev.iProduct)
        self.serial = ""
        if getattr(self.dev, 'iSerialNumber', 0):
            try:
                self.serial = usb.util.get_string(self.dev, self.dev.iSerialNumber) or ""
            except (usb.core.USBError, ValueError):
                self.serial = ""

        # Keep EP2 IN drained for the lifetime of the driver (see module docstring):
        # queued async transfers, or a blocking-read thread if those are unavailable.
        self._rx_thread = None
        try:
            self._usb_in = _AsyncBulkIn(self.dev, Bumblebee.EP_IN, Bumblebee.EP_SIZE,
                                        self._on_usb_data, self._on_usb_error)
        except Exception:
            self._usb_in = None
            self._rx_thread = threading.Thread(target=self._rx_loop, name="bumblebee-rx", daemon=True)
            self._rx_thread.start()

        # Soft-reset the firmware's USB FIFO/parser state; proves the link works.
        # Retried because a toggle mismatch left by an earlier host session drops one packet.
        self.init_attempts = 0
        timeout, self.timeout = self.timeout, 0.5
        try:
            while True:
                self.init_attempts += 1
                self.send_message(Bumblebee.CMD_INIT, bytes([]))
                if self.wait_for_ack(Bumblebee.CMD_INIT_ACK):
                    break
                if self.init_attempts >= 3:
                    self._stop_rx()
                    raise KBInterfaceError("Bumblebee did not ACK CMD_INIT")
        finally:
            self.timeout = timeout

    def _on_usb_data(self, pkt):
        """
        Strip one 64-byte IN packet's [chunk_len] header and append the chunk
        to rx_buffer.
        """
        if len(pkt) == 0:
            return
        n = min(pkt[0], len(pkt) - 1)
        if n > 0:
            with self._rx_lock:
                self.rx_buffer += bytes(pkt[1:1+n])
                self._rx_lock.notify_all()

    def _on_usb_error(self, e):
        with self._rx_lock:
            self._rx_error = e
            self._rx_lock.notify_all()

    def _rx_loop(self):
        """
        Fallback reader thread (blocking reads).
        """
        while not self._rx_stop.is_set():
            try:
                pkt = self.dev.read(Bumblebee.EP_IN, Bumblebee.EP_SIZE, 100)
            except usb.core.USBError as e:
                if e.errno in (errno.ETIMEDOUT, 60):
                    continue
                self._on_usb_error(e)
                return
            self._on_usb_data(pkt)

    def _stop_rx(self):
        self._rx_stop.set()
        if self._usb_in is not None:
            self._usb_in.stop()
        if self._rx_thread is not None and self._rx_thread.is_alive() and self._rx_thread is not threading.current_thread():
            self._rx_thread.join(1.0)

    def process_packet(self):
        """
        Process incoming packets from device (dongle)
        """
        while True:
            with self._rx_lock:
                buf = self.rx_buffer
                if len(buf) == 0:
                    return
                pkt_len = buf[0]
                if pkt_len < 3:
                    # Not a valid length byte: resync on the next byte
                    self.rx_buffer = buf[1:]
                    continue
                if len(buf) < pkt_len:
                    return
                if buf[pkt_len-1] != self.crc(buf[:pkt_len-1]):
                    # Bad checksum: resync on the next byte
                    self.rx_buffer = buf[1:]
                    continue
                payload = buf[1:pkt_len-1]
                self.rx_buffer = buf[pkt_len:]
            yield CommProtocolPacket(payload[0], payload[1:])

    def crc(self, x):
        """
        Compute CRC (sort of ;) for a given byte array.
        """
        c=0xff
        for i in x:
            c ^= i
        return c

    def process_rx(self, timeout=0):
        """
        Wait up to timeout seconds for data from the reader thread.
        Raises if the reader stopped on a USB error (dongle reset or unplugged).
        """
        with self._rx_lock:
            if self._rx_error is None and len(self.rx_buffer) == 0 and timeout > 0:
                self._rx_lock.wait(timeout)
            if self._rx_error is not None and len(self.rx_buffer) == 0:
                raise KBInterfaceError("Bumblebee USB read failed (dongle reset or unplugged?): %s" % (self._rx_error,))

    def send_message(self, command, data):
        """
        Send a message to our dongle.
        """
        # Without the reader thread a sniffing dongle would hang and reset.
        if self._rx_stop.is_set():
            raise KBInterfaceError("Bumblebee driver is closed")
        length = len(data) + 3
        buf = struct.pack('<BB', length, command) + data
        buf += bytes([ self.crc(buf) ])

        # Cut buffer in blocks of 64 bytes
        nbulks = int(len(buf)/64)
        if nbulks*64 < len(buf):
          nbulks += 1

        # Send each bulk separately
        sent=0
        for i in range(nbulks):
          sent += self.dev.write(Bumblebee.EP_OUT, buf[i*64:(i+1)*64])

        # Return total number of bytes sent
        return sent

    def send_packet(self, packet):
        """
        Send a 802.11.4 packet, FCS will be automatically added.
        FCS must not be provided, only packet data. Also, no need to add two
        bytes at the end of the packet to insert FCS.
        """
        self.last_tx_status = None
        self.send_message(
          Bumblebee.CMD_SEND_PKT,
          packet
        )
        if not self.wait_for_ack(Bumblebee.CMD_SEND_PKT_ACK):
            return False
        status = self._last_ack.get_data()
        if len(status) == 0:
            # Stock firmware: the ACK does not say whether the frame went on air.
            return True
        self.last_tx_status = status[0]
        return self.last_tx_status == Bumblebee.TX_OK


    def _do_set_channel(self):
        """
        Set channel for our dongle.
        """
        self.send_message(
          Bumblebee.CMD_SET_CHANNEL,
          bytes([ self._channel ])
        )
        if not self.wait_for_ack(Bumblebee.CMD_SET_CHANNEL_ACK):
            raise KBInterfaceError("Bumblebee did not ACK SET_CHANNEL %d" % self._channel)
        return True


    def _do_sniffer_on(self):
        """
        Enable sniffer
        """
        self.send_message(
          Bumblebee.CMD_SNIFF_ON,
          bytes([])
        )
        if not self.wait_for_ack(Bumblebee.CMD_SNIFF_ON_ACK):
            raise KBInterfaceError("Bumblebee did not ACK SNIFF_ON")
        return True


    def _do_sniffer_off(self):
        """
        Disable sniffer
        """
        self.send_message(
          Bumblebee.CMD_SNIFF_OFF,
          bytes([])
        )
        if not self.wait_for_ack(Bumblebee.CMD_SNIFF_OFF_ACK):
            raise KBInterfaceError("Bumblebee did not ACK SNIFF_OFF")
        return True


    def wait_for_ack(self, command_ack):
        """
        Wait for a specific ack. Captured frames that arrive meanwhile are
        kept for pnext().
        """
        entry_time = time.time()
        while True:
            # Check packets
            for pkt in self.process_packet():
              if pkt.get_command() == command_ack:
                self._last_ack = pkt
                return True
              if pkt.get_command() == Bumblebee.CMD_GOT_PKT:
                self._frames.append(pkt)

            # Timeout expired ?
            remaining = self.timeout - (time.time() - entry_time)
            if remaining <= 0:
              return False

            # Wait for the reader thread
            self.process_rx(min(remaining, 0.05))


    def close(self):
        """
        Close device.
        """
        try:
            if self.__stream_open == True and self._rx_error is None:
                self.sniffer_off()
        finally:
            self._stop_rx()
            usb.util.dispose_resources(self.dev)

    def check_capability(self, capab):
        return self.capabilities.check(capab)

    def get_capabilities(self):
        return self.capabilities.getlist()

    def __set_capabilities(self):
        """
        Sets the capability information appropriate for CC2531 Bumblebee FW.
        @rtype: None
        @return: None
        """
        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        self.capabilities.setcapab(KBCapabilities.INJECT, True)

    # KillerBee expects the driver to implement this function
    def get_dev_info(self):
        """
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        """
        # Firmware that sets iSerialNumber reports the chip IEEE address; 2.1r1 has none.
        return [self.name, "BUMBLEBEE", self.serial]

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel=None, page=0):
        """
        Turns the sniffer on such that pnext() will start returning observed data.
        Will set the command mode to Air Capture if it is not already set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        """
        self.capabilities.require(KBCapabilities.SNIFF)

        if self.dev is not None:
            if channel is not None:
                self.set_channel(channel, page)

            # Enable sniffer
            self._do_sniffer_on()
            self.__stream_open = True
        else:
            raise Exception('Device not connected')

    # KillerBee expects the driver to implement this function
    def sniffer_off(self):
        """
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        """
        if self.dev is not None:
            self._do_sniffer_off()
            self.__stream_open = False
        else:
            raise Exception('Device not connected')

    # KillerBee expects the driver to implement this function
    def set_channel(self, channel, page=0):
        """
        Sets the radio interface to the specifid channel (limited to 2.4 GHz channels 11-26)
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        """
        self.capabilities.require(KBCapabilities.SETCHAN)

        if page:
            raise Exception('SubGHz not supported')
        if 11 <= channel <= 26:
            self._channel = channel
            if self.dev is not None:
                self._do_set_channel()
        else:
            raise Exception('Invalid channel')


    # KillerBee expects the driver to implement this function
    def inject(self, packet, channel=None, count=1, delay=0, page=0):
        """
        Injects the specified packet contents.
        @type packet: String
        @param packet: Packet contents to transmit, without FCS.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @type count: Integer
        @param count: Transmits a specified number of frames, def=1
        @type delay: Float
        @param delay: Delay between each frame, def=1
        @rtype: None
        """
        if len(packet) < 1:
            raise Exception('Empty packet')
        if len(packet) > 125:                   # 127 - 2 to accommodate FCS
            raise Exception('Packet too long')

        if channel != None:
            self.set_channel(channel, page)

        for pnum in range(0, count):
            self.send_packet(packet)
            time.sleep(delay)

    # KillerBee expects the driver to implement this function
    def pnext(self, timeout=100):
        """
        Returns a dictionary containing packet data, else None.
        @type timeout: Integer
        @param timeout: Time to wait for packet reception in milliseconds
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a dictionary is returned with the keys bytes (string of packet bytes), validcrc (boolean if a vaid CRC), rssi (unscaled RSSI), and location (may be set to None). For backwards compatibility, keys for 0,1,2 are provided such that it can be treated as if a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        """
        if self.__stream_open == False:
            self.sniffer_on() #start sniffing

        deadline = time.time() + timeout / 1000.0
        packet = None
        while packet is None:
            if self._frames:
                packet = self._frames.popleft()
                break
            for pkt in self.process_packet():
                if pkt.get_command() == Bumblebee.CMD_GOT_PKT:
                    packet = pkt
                    break
            if packet is not None:
                break
            remaining = deadline - time.time()
            if remaining <= 0:
                return None
            self.process_rx(min(remaining, 0.05))

        payload = packet.get_data()
        if len(payload) < 3:
            return None

        # CC2531 only allow (for the moment) to capture packets with valid CRC
        validcrc = True

        # Extract RSSI and LQI from payload buffer.
        rssi = struct.unpack('<b', bytes([payload[0]]))[0]
        correlation = struct.unpack('<b', bytes([payload[1]]))[0]

        ret = {1:validcrc, 2:rssi,
                    'validcrc':validcrc, 'rssi':rssi, 'lqi':correlation,
                    'dbm':rssi,'datetime':datetime.utcnow()}

        # Convert the framedata to a string for the return value, and replace the TI FCS with a real FCS
        # if the radio told us that the FCS had passed validation.
        if validcrc:
            ret[0] = bytearray_to_bytes(payload[2:]) + makeFCS(payload[2:])
        else:
            ret[0] = bytearray_to_bytes(payload)
        ret['bytes'] = ret[0]
        return ret

    def jammer_on(self, channel=None, page=0):
        """
        Not yet implemented.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        """
        raise Exception('Not yet implemented')

    def set_sync(self, sync=0xA7):
        """
        Set the register controlling the 802.15.4 PHY sync byte.
        """
        raise Exception('Not yet implemented')

    def jammer_off(self, channel=None, page=0):
        """
        Not yet implemented.
        @return: None
        @rtype: None
        """
        raise Exception('Not yet implemented')

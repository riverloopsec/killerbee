"""
CC2530/CC2531 injection support is contributed by virtualabs.

This driver communicates with a CC2531 compatible USB dongle
flashed with 'Bumblebee' firmware. It is then able to communicate
with this dongle through USB (bulk data transfer).

Bumblebee firmware provides 2.4GHz sniffing and injection capabilities,
with RSSI/LQI. Bad FCS sniffing and jamming are not currently supported.
"""

from __future__ import print_function # type: ignore
import sys # type: ignore
import struct # type: ignore
import time # type: ignore
from array import array # type: ignore
from datetime import datetime # type: ignore
from .kbutils import KBCapabilities, makeFCS, bytearray_to_bytes # type: ignore

import usb.core # type: ignore
import usb.util # type: ignore

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


    def __init__(self, dev, bus):
        """
        Initialize device and capabilities.
        """

        self.dev = dev
        self.rx_buffer = bytes()
        self.usb_rx_buffer = array('B', b'\x00'*256)
        self._channel = None
        self.__stream_open = False
        self.timeout = 2.0
        self.capabilities = KBCapabilities()
        self.__set_capabilities()

        # Set configuration
        self.dev.set_configuration()

        self.name = usb.util.get_string(self.dev, self.dev.iProduct)

    def process_packet(self):
        """
        Process incoming packets from device (dongle)
        """
        # Did we receive some data ?
        if len(self.rx_buffer) > 0:

            pkt_len = self.rx_buffer[0]

            # Loop on received packets and yield CommProtocolPacket objects
            while (len(self.rx_buffer) >= pkt_len) and (pkt_len > 0):
                # Extract payload
                payload = self.rx_buffer[1:pkt_len-1]
                
                # Extract CRC
                crc = self.rx_buffer[pkt_len-1]

                # Check CRC
                if crc == self.crc(self.rx_buffer[:pkt_len-1]):
                    # Chomp packet
                    self.rx_buffer = self.rx_buffer[pkt_len:]

                    # Yield packet
                    yield CommProtocolPacket(payload[0], payload[1:]) 
                else:
                    # Chomp packet
                    self.rx_buffer = self.rx_buffer[pkt_len:]

                # Process next packet if any
                if len(self.rx_buffer) > 0:
                  pkt_len = self.rx_buffer[0]
                else:
                  pkt_len = 0

    def crc(self, x):
        """
        Compute CRC (sort of ;) for a given byte array.
        """
        c=0xff
        for i in x:
            c ^= i
        return c

    def process_rx(self):
        """
        Read incoming data and fill RX buffer.
        """
        try:
          nbytes = self.dev.read(Bumblebee.EP_IN, self.usb_rx_buffer, 100)
          if nbytes > 0:
            self.rx_buffer += self.usb_rx_buffer.tobytes()[:nbytes]
        except usb.core.USBError as e:
            if e.errno is not 110 and e.errno is not 60: #Operation timed out
                print("Error args: {}".format(e.args))
                raise e
                #TODO error handling enhancements for USB 1.0
            else:
                return None

    def send_message(self, command, data):
        """
        Send a message to our dongle.
        """
        length = len(data) + 3
        buf = struct.pack('<BB', length, command) + data
        buf += bytes([ self.crc(buf) ])

        # Cut buffer in blocks of 64 bytes
        nbulks = int(len(buf)/64)
        if nbulks*64 < len(buf):
          nbulks += 1

        # Send each bulk separately
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
        self.send_message(
          Bumblebee.CMD_SEND_PKT,
          packet
        )
        return self.wait_for_ack(Bumblebee.CMD_SEND_PKT_ACK)


    def _do_set_channel(self):
        """
        Set channel for our dongle.
        """
        self.send_message(
          Bumblebee.CMD_SET_CHANNEL,
          bytes([ self._channel ])
        )
        return self.wait_for_ack(Bumblebee.CMD_SET_CHANNEL_ACK)


    def _do_sniffer_on(self):
        """
        Enable sniffer
        """
        self.send_message(
          Bumblebee.CMD_SNIFF_ON,
          bytes([])
        )
        return self.wait_for_ack(Bumblebee.CMD_SNIFF_ON_ACK)


    def _do_sniffer_off(self):
        """
        Disable sniffer
        """
        self.send_message(
          Bumblebee.CMD_SNIFF_OFF,
          bytes([])
        )
        return self.wait_for_ack(Bumblebee.CMD_SNIFF_OFF_ACK)


    def wait_for_ack(self, command_ack):
        """
        Wait for a specific ack.
        """
        entry_time = time.time()
        while True:
            # Poll our USB device
            self.process_rx()

            # Check packets
            for pkt in self.process_packet():
              if pkt.get_command() == command_ack:
                return True

            # Timeout expired ?
            if (time.time() - entry_time) >= self.timeout:
              return False

            # Wait a bit
            time.sleep(0.1)


    def close(self):
        """
        Close device.
        """
        if self.__stream_open == True:
            self.sniffer_off()
            pass

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
        # TODO Determine if there is a way to get a unique ID from the device
        return [self.name, "BUMBLEBEE", ""]

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

        if channel >= 11 or channel <= 26:
            self._channel = channel
            if self.dev is not None:
                self._do_set_channel()
        else:
            raise Exception('Invalid channel')
        if page:
            raise Exception('SubGHz not supported')


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
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a dictionary is returned with the keys bytes (string of packet bytes), validcrc (boolean if a vaid CRC), rssi (unscaled RSSI), and location (may be set to None). For backwards compatibility, keys for 0,1,2 are provided such that it can be treated as if a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        """
        if self.__stream_open == False:
            self.sniffer_on() #start sniffing

        # Fetch incoming data
        self.process_rx()

        # Loop on all received packets
        for packet in self.process_packet():
            if packet is not None:
                payload = packet.get_data()
            else:
                return None

            # CC2531 only allow (for the moment) to capture packets with valid CRC
            validcrc = True

            # Extract RSSI and LQI from payload buffer.
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

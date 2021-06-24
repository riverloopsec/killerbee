'''
Support for the Silabs based version of "BeeHive" SNIFFER/ INJECTOR firmware
Author: Adam Laurie <adam@algroup.co.uk> aka RFIDiot

'''

import usb # type: ignore
import serial # type: ignore
import time # type: ignore
import struct # type: ignore
from datetime import datetime, date # type: ignore
from datetime import time as dttime # type: ignore
from .kbutils import KBCapabilities, makeFCS # type: ignore

MODE_NONE    = 0x01
MODE_SNIFF   = 0x02

class SL_BEEHIVE:
    def __init__(self, serialpath):
        '''
        Instantiates the KillerBee class for Silabs BEEHIVE
        @param serialpath:  /dev/ttyACM* type serial port identifier
        @return: None
        @rtype: None
        '''
        self._channel = None
        self._page = 0
        self.handle = None
        self.dev = serialpath
        self.date = None
        self.lon, self.lat, self.alt = (None, None, None)
        self.__stream_open = False
        self.handle = serial.Serial(port=self.dev, baudrate=115200, \
                                    timeout=.02, bytesize=8, parity='N', stopbits=1, xonxoff=0)
        self.capabilities = KBCapabilities()
        self.__set_capabilities()

    def close(self):
        '''
        Closes the serial port. After closing, must reinitialize class again before use.
        @return: None
        @rtype: None
        '''
        self.handle.close()
        self.handle = None

    # KillerBee implements these, maybe it shouldn't and instead leave it to the driver as needed.
    # TODO deprecate these functions in favor of self.capabilities class functions
    def check_capability(self, capab):
        return self.capabilities.check(capab)
    def get_capabilities(self):
        return self.capabilities.getlist()

    def __set_capabilities(self):
        '''
        Sets the capability information for Silabs Node Test
        @rtype: None
        @return: None
        '''
        #NOTE frequencies will actually depend on radio daughterboard on the Silabs dev kit
        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.FREQ_915, True)
        self.capabilities.setcapab(KBCapabilities.FREQ_863, True)
        self.capabilities.setcapab(KBCapabilities.FREQ_868, True)
        self.capabilities.setcapab(KBCapabilities.FREQ_870, True)
        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.INJECT, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        return

    def get_dev_info(self):
        '''
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return [self.dev, "BeeHive SG", ""]

    def __send_cmd(self, cmdstr, arg=None, confirm= True, send_return= True, extra_delay= 0, initial_read= 3):
        # read any cruft
        #time.sleep(0.1)
        for x in range(initial_read):
            self.handle.readline()

        # some commands require us to be in idle, so do it always
        self.handle.write("rx 0\r")
        time.sleep(0.02)
        for x in range(3):
            self.handle.readline()

        if arg != None:
            cmdstr += ' ' + arg
        self.handle.write(cmdstr)
        if send_return:
            self.handle.write('\r')
        #time.sleep(0.1)
        time.sleep(extra_delay)
        if confirm:
            ret= False
            for x in range(100):
                d= self.handle.readline().strip()
                if d[-1:] == '>':
                    ret= True
                    break
        else:
            ret= True

        if not ret:
            raise Exception('Could not send command %s' % cmdstr)

    def __dissect_pkt(self, packet):
        '''
        internal routine to deconstruct serial text received from device
        packet will be in the format: "{{(rxPacket)}{len:11}{timeUs:994524212}{crc:Pass}{rssi:-44}{lqi:210}{phy:0}{isAck:False}{syncWordId:0}{antenna:0}{payload: 0x0a 0x03 0x08 0xba 0xff 0xff }"
        '''
        try:
            data = packet.replace('}','').split(':')[10].split()
        except:
            return None, None, None
        if not data:
            return None, None, None
        # payload is in the form e.g. "0x03 0x08 0xA3 0xFF 0xFF 0xFF 0xFF 0x07" so we need to convert to a string
        out = ''
        # first byte is length
        if len(data) != int(data[0], 16) + 1:
            return None, None, None
        for x in data[1:]:
            try:
                out += chr(int(x, 16))
            except:
                return None, None, None
        try:
            crc = packet.replace('}','').split(':')[4].split('{')[0] == 'Pass'
            rssi = packet.replace('}','').split(':')[4].split('{')[0]
        except:
            return None, None, None
        return rssi, out, crc

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel=None, page=0):
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.  Will set the command mode to Air Capture if it is not already
        set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SNIFF)

        if channel != None or page:
            self.set_channel(channel, page)

        if not self.__stream_open:
            # make sure we get all packets
            self.__init_radio()
            # start sniffer
            self.__send_cmd("rx", "1", confirm= False)
            for x in range(5):
                d = self.handle.readline()
                if 'Rx:Enabled' in d:
                    self.mode = MODE_SNIFF
                    self.__stream_open = True

        if not self.__stream_open:
             raise Exception('Could not start sniffer')

    # KillerBee expects the driver to implement this function
    def sniffer_off(self):
        '''
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        '''
        # reset timeout as sniffer has made it long
        self.handle.timeout= 0.2
        self.__send_cmd("rx", "0", confirm= False, initial_read= 0)
        for x in range(3):
            d= self.handle.readline().strip()
            if "Rx:Disabled" in d:
                self.mode = MODE_NONE
                self.__stream_open = False
                self.handle.readline()
        if self.__stream_open:
            raise Exception('Could not stop sniffer')

    def __init_radio(self):
        self.__send_cmd("reset", extra_delay = 0.6)
        self.__send_cmd("rx", "0")
        self.set_channel(self._channel, self._page)
        self.__send_cmd("BeeHiveMode", "1")
        self.__send_cmd("BeeHiveOptions", "1 1 1 1")

    # KillerBee expects the driver to implement this function
    def set_channel(self, channel, page=0):
        '''
        Sets the radio interface to the specifid channel (limited to 2.4 GHz channels 11-26)
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SETCHAN)

        #subghz channels must be calculated to include page (3 LSB page + 5 LSB channel)
        # e.g. page 28 channel 15 == '100' + '01111' = '10001111' = 0x8F
        self._channel = channel
        self._page = page
        if page:
            channel = ((page << 5) & 0xff) + (channel & 0x1f)
            if page == 31:
                self.__send_cmd("915MHz", extra_delay= 0.3)
            else:
                self.__send_cmd("863MHz", extra_delay= 0.3)
        else:
            self.__send_cmd("2p4GHz")
        #TODO actually check that it responds correctly to the request
        self.__send_cmd("setchannel", "%d" % channel)

    # KillerBee expects the driver to implement this function
    # note that BEEHIVE never generates FCS - you must provide it as part of the packet!
    def inject(self, packet, channel=None, count=1, delay=0, page=0):
        '''
        Injects the specified packet contents.
        @type packet: String
        @param packet: Packet contents to transmit, with FCS.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type count: Integer
        @param count: Transmits a specified number of frames, def=1
        @type delay: Float
        @param delay: Delay between each frame, def=1
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.INJECT)

        if len(packet) < 1:
            raise Exception('Empty packet')
        if len(packet) > 127:
            raise Exception('Packet too long')

        if channel != None or page:
            self.set_channel(channel, page)

        # 
        self.__send_cmd("setTxLength", "%d" % len(packet))
        maxp = 118
        # we can send max 256 bytes over the wire so we must split large packets due to hex doubling the size
        if len(packet) > maxp:
            tosend = maxp
        else:
            tosend = len(packet) 
        self.__send_cmd("setTxPayload", "00 %02x%s" % ((len(packet)), packet[:tosend].encode('hex')))
        if len(packet) > maxp:
            self.__send_cmd("setTxPayload", "%d %s" % (tosend + 1, packet[tosend:].encode('hex')))
        for pnum in range(0, count):
            self.__send_cmd("tx", "1", confirm= False)
            time.sleep(delay)

    # KillerBee expects the driver to implement this function
    #TODO I suspect that if you don't call this often enough while getting frames, the serial buffer may overflow.
    def pnext(self, timeout=1):
        '''
        Returns packet data as a string, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received,
                 a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''
        if not self.__stream_open:
            self.sniffer_on() #start sniffing

        self.handle.timeout=timeout         # Allow pySerial to handle timeout
        packet = self.handle.readline().strip()
        if packet == '':
            return None   # Sense timeout case and return

        rssi, frame, validcrc = self.__dissect_pkt(packet)
        if not frame:
            print("Error parsing stream received from device:", packet)

        # Parse received data as <rssi>!<time>!<packtlen>!<frame>
        try:
            rssi = int(rssi)
        except:
            print("Error parsing stream received from device:", packet)
            return None
        #Return in a nicer dictionary format, so we don't have to reference by number indicies.
        #Note that 0,1,2 indicies inserted twice for backwards compatibility.
        result = {0:frame, 1:validcrc, 2:rssi, 'bytes':frame, 'validcrc':validcrc, 'rssi':rssi}
        result['dbm'] = None #TODO calculate dBm antenna signal based on RSSI formula
        result['datetime'] = datetime.utcnow() # TODO - see what time field in sniff is actually telling us
        result['location'] = (self.lon, self.lat, self.alt)
        return result

    def ping(self, da, panid, sa, channel=None, page=0):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        raise Exception('Not yet implemented')

    def jammer_on(self, channel=None, page=0):
        '''
        Not yet implemented.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.PHYJAM)

        if channel != None or page:
            self.set_channel(channel, page)

        #TODO implement
        raise Exception('Not yet implemented')

    def jammer_off(self, channel=None, page=0):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        #TODO implement
        raise Exception('Not yet implemented')

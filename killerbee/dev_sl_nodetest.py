'''
Support for the Silabs Node Test firmware (https://www.silabs.com/documents/public/application-notes/AN1019-NodeTest.pdf)
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

class SL_NODETEST:
    def __init__(self, serialpath):
        '''
        Instantiates the KillerBee class for Silabs Node Test
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
                                    timeout=.1, bytesize=8, parity='N', stopbits=1, xonxoff=0)
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
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        return

    def get_dev_info(self):
        '''
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return [self.dev, "Silabs Node Test", ""]

    def __send_cmd(self, cmdstr, arg=None, confirm= True, send_return= True):
        self.handle.flush()
        #time.sleep(1.5) #this delay seems crucial

        if arg != None:
            cmdstr += ' ' + arg
        self.handle.write(cmdstr.encode())
        if send_return:
            self.handle.write('\r'.encode())
        if confirm:
            ret= False
            for x in range(10):
                d= self.handle.readline().strip()
                if d == b'>':
                    ret= True
                    break
        else:
            ret= True

        if not ret:
            raise Exception('Could not send command %s' % cmdstr)

    def __dissect_pkt(self, packet):
        '''
        internal routine to deconstruct serial text received from device
        packet will be in the format: "{{num}   {oflo}  {seq}   {per}  {err} {lqi}  {rssi}{ed}   {gain}       {status} {time}       {fp}{length}{payload}}"
        @type packet: String
        @param: packet: The raw packet to dissect
        @rtype: List
        @return: Returns None if packet is not in correct format.  When a packet is correct,
                 a list is returned, in the form [ String: Frame | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''
        data = packet[1:].decode().replace('{',' ').replace('}',' ').split()
        # should be 12 fields + payload length + payload
        if not data or not len(data[13:]) == int(data[12], 16):
            print("Error parsing stream received from device (payload size error):", packet)
            return None
        # payload is in the form e.g. "0x03 0x08 0xA3 0xFF 0xFF 0xFF 0xFF 0x07" so we need to convert to a string
        frame = b''
        for x in data[13:]:
            try:
                frame += chr(int(x, 16)).encode('latin-1')
            except:
                print("Error parsing stream received from device (invalid payload):", packet)
                return None

        # Parse other useful fields
        try:
            rssi = int(data[6])
            # sniffer doesn't give us the CRC so we must add it, but must be correct or we would not have received it
            validcrc = True
            frame += makeFCS(frame)
        except:
            print("Error parsing stream received from device (invalid rssi or FCS build error):", packet)
            return None
        return [frame, validcrc, rssi]

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
            # make sure we get hex dump of RX packet
            self.__send_cmd("showPayload", "1")
            # start sniffer
            self.__send_cmd("rx", confirm= False)
            for x in range(10):
                d = self.handle.readline()
                if b'{payload}' in d:
                    self.mode = MODE_SNIFF
                    self.__stream_open = True
                    break

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
        self.__send_cmd("e", send_return= False, confirm= False)
        for x in range(5):
            d= self.handle.readline().strip()
            if b"test end" in d:
                self.mode = MODE_NONE
                self.__stream_open = False
                break
        if self.__stream_open:
            raise Exception('Could not stop sniffer')

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
        if page:
            channel = ((page << 5) & 0xff) + (channel & 0x1f)
        self._page = page
        #TODO actually check that it responds correctly to the request
        self.__send_cmd("setchannel", "%02x" % channel)

    # KillerBee expects the driver to implement this function
    def inject(self, packet, channel=None, count=1, delay=0, page=0):
        '''
        Injects the specified packet contents.
        @type packet: String
        @param packet: Packet contents to transmit, without FCS.
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
        if len(packet) > 125:                   # 127 - 2 to accommodate FCS
            raise Exception('Packet too long')

        if channel != None or page:
            self.set_channel(channel, page)

        # Append two bytes to be replaced with FCS by firmware.
        packet = ''.join([packet, "\x00\x00"])

        raise Exception('Not yet implemented')

    # KillerBee expects the driver to implement this function
    #TODO I suspect that if you don't call this often enough while getting frames, the serial buffer may overflow.
    def pnext(self, timeout=100):
        '''
        Returns packet data as a string, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None if timeout expires and no packet received or packet is corrupt.  When a packet is received,
                 a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''
        if not self.__stream_open:
            self.sniffer_on() #start sniffing

        self.handle.timeout=timeout         # Allow pySerial to handle timeout
        packet = self.handle.readline().strip()
        if packet == '':
            return None   # Sense timeout case and return

        data = self.__dissect_pkt(packet)
        if not data:
            return None
        frame = data[0]
        validcrc = data[1]
        rssi = data[2]
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

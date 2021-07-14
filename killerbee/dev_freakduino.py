'''
Support from the Freakduino platform from Abika/Freaklabs.

This is not a maintained platfrom and functionality may be broken or lacking.
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

class FREAKDUINO:
    def __init__(self, serialpath):
        '''
        Instantiates the KillerBee class for our sketch running on ChibiArduino on Freakduino hardware.
        @param serialpath:  /dev/ttyUSB* type serial port identifier
        @return: None
        @rtype: None
        '''
        self._channel = None
        self._page = 0
        self.handle = None
        self.dev = serialpath
        self.date = None
        self.lon, self.lat, self.alt = (None, None, None)
        self.handle = serial.Serial(port=self.dev, baudrate=57600, \
                                    timeout=1, bytesize=8, parity='N', stopbits=1, xonxoff=0)
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
        Sets the capability information for Freakdruino device based on the currently loaded sketch.
        @rtype: None
        @return: None
        '''
        #TODO have it set based on what the sketch says it can support
        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        return

    def get_dev_info(self):
        '''
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return [self.dev, "Dartmouth Freakduino", ""]

    def __send_cmd(self, cmdstr, arg=None):
        self.handle.flush()
        time.sleep(1.5) #this delay seems crucial

        self.handle.write("S")
        self.handle.write(cmdstr)
        if arg != None: self.handle.write(arg)
        self.handle.write('\r')

        self.handle.flush()

    # Deprecated due to unreliability
    def __serial_cmd(self, cmdstr, arg=None):
        '''
        Sends a command over the self.conn serial connection.
        Ex: If provided cmdstr = "C!N" it will send "SC!N", telling the device to turn on sniffing ("N"),
        and it expects to receive a confirmation back "&C!N" to confirm success.
        '''
        print("Flushing out of buffer:", self.handle.inWaiting())
        self.handle.flushInput()
        if len(cmdstr) > 3:
            raise Exception("Command string is less than minimum length (S%s)." % cmdstr)
        self.__send_cmd(cmdstr, arg)

        # TODO ugly and unreliable:
        # This should just wait for a & and then parse things after it,
        # however it seems sometimes you have to resend the command or something.
        print("Line:", self.handle.readline(eol='&'))
        counter = 0
        char = self.handle.read()
        while (char != '&'):
            print(self.handle.inWaiting(), "Waiting...", char)
            time.sleep(0.01)
            if (counter > 8):
                self.__send_cmd(cmdstr, arg)
                counter = 0
                print("Resend Response Line:", self.handle.readline(eol='&'))
            else: counter += 1
            char = self.handle.read()
        response = ''
        for i in range(3):
            response += self.handle.read()

        if response == cmdstr[:3]:
            print("Got a response:", response, "matches", cmdstr)
            return True
        else:
            print("Invalid response:", response, cmdstr[:3])
            return False

    # Send the command for the Dartmouth-mod Freakduino to dump data logged in EEPROM
    def eeprom_dump(self):
        self.__send_cmd("C!D")

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel=None, page=0):
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.  Will set the command mode to Air Capture if it is not already
        set.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SNIFF)

        if channel != None:
            self.set_channel(channel, page)

        #TODO implement mode change to start sniffer sending packets to us
        self.__send_cmd("C!N")
        self.mode = MODE_SNIFF
        self.__stream_open = True

    # KillerBee expects the driver to implement this function
    def sniffer_off(self):
        '''
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        '''
        self.__send_cmd("C!F")
        self.mode = MODE_NONE
        self.__stream_open = False

    # KillerBee expects the driver to implement this function
    def set_channel(self, channel, page=0):
        '''
        Sets the radio interface to the specifid channel (limited to 2.4 GHz channels 11-26)
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SETCHAN)

        if channel >= 10 or channel <= 26:
            self._channel = channel
            #TODO actually check that it responds correctly to the request
            self.__send_cmd("C!C %d" % channel)
        else:
            raise Exception('Invalid channel')
        if page:
            raise Exception('SubGHz not supported')

    # KillerBee expects the driver to implement this function
    def inject(self, packet, channel=None, count=1, delay=0, page=0):
        '''
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
        '''
        self.capabilities.require(KBCapabilities.INJECT)

        if len(packet) < 1:
            raise Exception('Empty packet')
        if len(packet) > 125:                   # 127 - 2 to accommodate FCS
            raise Exception('Packet too long')

        if channel != None:
            self.set_channel(channel, page)

        # Append two bytes to be replaced with FCS by firmware.
        packet = ''.join([packet, "\x00\x00"])

        for pnum in range(0, count):
            raise Exception('Not yet implemented')
            # Format for packet is opcode CMD_INJECT_FRAME, one-byte length,  packet data
            #TODO RZ_USB_COMMAND_EP, struct.pack("B", RZ_CMD_INJECT_FRAME) + struct.pack("B", len(packet)) + packet)
            time.sleep(delay)

    # KillerBee expects the driver to implement this function
    #TODO I suspect that if you don't call this often enough while getting frames, the serial buffer may overflow.
    def pnext(self, timeout=100):
        '''
        Returns packet data as a string, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received,
                 a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''
        if self.__stream_open == False:
            self.sniffer_on() #start sniffing
        return self.pnext_rec(timeout)

    # Bulk of pnext implementation, but does not ensure the sniffer is on first, thus usable for EEPROM reading
    def pnext_rec(self, timeout=100):
        pdata = ''
        ldata = ''

        self.handle.timeout=timeout         # Allow pySerial to handle timeout
        startChar = self.handle.read()
        if startChar == None: return None   # Sense timeout case and return

        # Listens for serial message of general format R!<data>;
        if startChar == "R":                # Get packet data
            if self.handle.read() == "!":
                x = self.handle.read()
                while (x != ";"):
                    pdata += x
                    x = self.handle.read()
        if startChar == "L":                # Get location data
            if self.handle.read() == "!":
                x = self.handle.read()
                while (x != ";"):
                    ldata += x
                    x = self.handle.read()
        if startChar == "[":                # Sense when done reading from EEPROM
            if self.handle.read(40) == "{[ DONE READING BACK ALL LOGGED DATA ]}]":
                raise StopIteration("All Data Read")

        # If location received, update our local variables:
        if ldata != None and ldata != '':
            self.processLocationUpdate(ldata)

        if pdata == None or pdata == '':
            return None

        # Parse received data as <rssi>!<time>!<packtlen>!<frame>
        data = pdata.split("!", 3)
        try:
            rssi = ord(data[0])
            frame = data[3]
            if frame[-2:] == makeFCS(frame[:-2]): validcrc = True
            else: validcrc = False
        except:
            print("Error parsing stream received from device:", pdata, data)
            return None
        #Return in a nicer dictionary format, so we don't have to reference by number indicies.
        #Note that 0,1,2 indicies inserted twice for backwards compatibility.
        result = {0:frame, 1:validcrc, 2:rssi, 'bytes':frame, 'validcrc':validcrc, 'rssi':rssi}
        result['dbm'] = None #TODO calculate dBm antenna signal based on RSSI formula
        result['datetime'] = self.getCaptureDateTime(data)
        result['location'] = (self.lon, self.lat, self.alt)
        return result

    def getCaptureDateTime(self, data):
        try:
            timestr = "%08d" % (struct.unpack('L', data[1])[0]) #in format hhmmsscc
            time = dttime(int(timestr[:2]), int(timestr[2:4]), int(timestr[4:6]), int(timestr[6:]))
        except:
            print("Issue with time format:", timestr, data)
            time = None
        if self.date == None: self.date = date.utcnow().date()
        if time == None or time == dttime.min: time = (datetime.utcnow()).time()
        #TODO address timezones by going to UTC everywhere
        return datetime.combine(self.date, time)

    def processLocationUpdate(self, ldata):
        '''
        Take a location string passed from the device and update the driver's internal state of last received location.
        Format of ldata: longlatialtidate
        '''
        self.lon = struct.unpack('l', ldata[0:4])[0]
        self.lat = struct.unpack('l', ldata[4:8])[0]
        self.alt = struct.unpack('l', ldata[8:12])[0]
        date = str(struct.unpack('L', ldata[12:16])[0])
        self.date = datetime.date(date[-2:], date[-4:-2], date[:-4])
        #TODO parse data formats (lon=-7228745 lat=4370648 alt=3800 age=63 date=70111 time=312530)
        print(self.lon, self.lat, self.alt, self.date)

    def ping(self, da, panid, sa, channel=None, page=0):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        raise Exception('Not yet implemented')

    def jammer_on(self, channel=None, page=0, method=None):
        '''
        Not yet implemented.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.PHYJAM)

        if self.__cmdmode != RZ_CMD_MODE_AC:
            self._set_mode(RZ_CMD_MODE_AC)

        if channel != None:
            self.set_channel(channel, page)

        #TODO implement
        raise Exception('Not yet implemented')

    def jammer_off(self):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        #TODO implement
        raise Exception('Not yet implemented')

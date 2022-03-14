'''
GoodFET Chipcon RF Radio Client for ApiMote Hardware

(C) 2013 Ryan Speers <ryan at riverloopsecurity.com>

The ApiMote product is a work in progress.
This code is being rewritten and refactored.

TODO list (help is welcomed):
  - RF testing and calibration for RSSI/dBm
  - Testing carrier jamming and implementing jammer_off()
  - Platform recognition (ApiMote versons)
'''

from typing import Optional, Dict, Union, Any, List

import os
import time
import struct
import time 
from datetime import datetime, timedelta 
from .kbutils import KBCapabilities, makeFCS 
from .GoodFETCCSPI import GoodFETCCSPI

CC2420_REG_SYNC: int = 0x14

class APIMOTE:
    def __init__(self, dev: str) -> None:
        '''
        Instantiates the KillerBee class for the ApiMote platform running GoodFET firmware.
        @type dev:   String
        @param dev:  Serial device identifier (ex /dev/ttyUSB0)
        @return: None
        @rtype: None
        '''
        self.packet_queue: Optional[bytes] = None
        self.packet_queue_rssi: Optional[int] = None

        self._channel: Optional[int] = None
        self._page: int = 0
        self.handle: Optional[Any] = None
        self.dev: str = dev

        # Set enviroment variables for GoodFET code to use
        os.environ["platform"] = "apimote2"
        os.environ["board"] = "apimote2"

        self.handle = GoodFETCCSPI()
        self.handle.serInit(port=self.dev)
        self.handle.setup()

        self.__stream_open: bool = False
        self.capabilities: KBCapabilities = KBCapabilities()
        self.__set_capabilities()

    def close(self) -> None:
        
        if self.handle is None:
            raise Exception("Handle does not exist");

        self.handle.serClose()
        self.handle = None

    def check_capability(self, capab: int) -> bool:
        return self.capabilities.check(capab)

    def get_capabilities(self) -> Dict[int, bool]:
        return self.capabilities.getlist()

    def __set_capabilities(self) -> None:
        '''
        Sets the capability information appropriate for GoodFETCCSPI client and firmware.
        @rtype: None
        @return: None
        '''
        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        self.capabilities.setcapab(KBCapabilities.INJECT, True)
        self.capabilities.setcapab(KBCapabilities.PHYJAM_REFLEX, True)
        self.capabilities.setcapab(KBCapabilities.SET_SYNC, True)
        self.capabilities.setcapab(KBCapabilities.PHYJAM, True)
        return

    # KillerBee expects the driver to implement this function
    def get_dev_info(self) -> List[Union[str, Any]]:
        '''
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return [self.dev, "GoodFET Apimote v2", ""]

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel: Optional[int]=None, page: int=0) -> None:
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
        
        if self.handle is None:
            raise Exception("Handle does not exist")

        self.handle.RF_promiscuity(1)
        self.handle.RF_autocrc(0)

        if channel is not None:
            self.set_channel(channel, page)
        
        self.handle.CC_RFST_RX()

        self.__stream_open = True

    # KillerBee expects the driver to implement this function
    def sniffer_off(self) -> None:
        '''
        Turns the sniffer off, freeing the hardware for other functions.  It is
        not necessary to call this function before closing the interface with
        close().
        @rtype: None
        '''
        #TODO actually have firmware stop sending us packets!
        self.__stream_open = False

    # KillerBee expects the driver to implement this function
    def set_channel(self, channel: int, page: int=0) -> None:
        '''
        Sets the radio interface to the specifid channel (limited to 2.4 GHz channels 11-26)
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        if self.handle is None:
            raise Exception("Handle does not exist")

        self.capabilities.require(KBCapabilities.SETCHAN)

        if channel >= 11 and channel <= 26:
            self._channel = channel
            self.handle.RF_setchan(channel)
        else:
            raise Exception('Invalid channel')
        if page:
            raise Exception('SubGHz not supported')

    # KillerBee expects the driver to implement this function
    def inject(self, packet: bytes, channel: Optional[int]=None, count: int=1, delay: int=0, page: int=0) -> None:
        '''
        Injects the specified packet contents.
        @type packet: Bytes 
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
        if self.handle is None:
            raise Exception("Handle does not exist")

        self.capabilities.require(KBCapabilities.INJECT)

        if len(packet) < 1:
            raise Exception('Empty packet')
        if len(packet) > 125:                   # 127 - 2 to accommodate FCS
            raise Exception('Packet too long')

        if channel is not None:
            self.set_channel(channel, page)

        self.handle.RF_autocrc(1)               #let radio add the CRC
        for pnum in range(0, count):
            gfready = list(bytearray(packet))  #convert packet string to GoodFET expected integer format
            gfready.insert(0, len(gfready)+2)   #add a length that leaves room for CRC
            self.handle.RF_txpacket(gfready)
            time.sleep(1)

    # KillerBee expects the driver to implement this function
    def pnext(self, timeout: int=100) -> Any:
        '''
        Returns a dictionary containing packet data, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a dictionary is returned with the keys bytes (string of packet bytes), validcrc (boolean if a vaid CRC), rssi (unscaled RSSI), and location (may be set to None). For backwards compatibility, keys for 0,1,2 are provided such that it can be treated as if a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''

        if self.handle is None:
            raise Exception("Handle does not exist")

        if self.__stream_open == False:
            self.sniffer_on()
        
        if self.packet_queue is None:
            packet: Optional[bytes] = None
            start = datetime.now()

            while (packet is None) and ((start + timedelta(seconds=timeout)) > datetime.now()):
                packet = self.handle.RF_rxpacket()
                rssi = self.handle.RF_getrssi() #TODO calibrate

            if packet is None:
                return None

            if packet[0]+1 < len(packet):
                self.packet_queue = packet[packet[0]+1+1:]
                self.packet_queue_rssi = rssi
             
            frame = packet[1:packet[0]+1]

        else: 
            packet = self.packet_queue
            self.packet_queue = None
            frame = packet

            if self.packet_queue_rssi is None:
                rssi = None
            else:
                rssi = self.packet_queue_rssi


        validcrc: bool = False
        if frame[-2:] == makeFCS(frame[:-2]):
            validcrc = True
        #Return in a nicer dictionary format, so we don't have to reference by number indicies.
        #Note that 0,1,2 indicies inserted twice for backwards compatibility.
        result: Dict[Union[int, str], Any] = {
            0: frame, 
            1: validcrc, 
            2: rssi, 
            'bytes': frame, 
            'validcrc': validcrc, 
            'rssi': rssi, 
            'location': None
        }

        result['datetime'] = datetime.utcnow()
        if rssi is None:
            result['dbm'] = None
        else:          
            result['dbm'] = rssi - 45 #TODO tune specifically to the Apimote platform (does ext antenna need to different?)
        return result
 
    def ping(self, da: Any, panid: Any, sa: Any, channel: Optional[int]=None, page: int=0) -> None:
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        raise Exception('Not yet implemented')

    def jammer_on(self, channel: Optional[int]=None, page: int=0, method: Optional[str]=None) -> None:
        '''
        Implements reflexive jamming or constant carrier wave jamming.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @type page: Integer
        @param page: Sets the subghz page, not supported on this device
        @rtype: None
        '''

        if self.handle is None:
            raise Exception("Handle does not exist")

        if method is None: 
            method = "constant"

        if method == "reflexive":
            self.capabilities.require(KBCapabilities.PHYJAM_REFLEX)
        elif method == "constant":
            self.capabilities.require(KBCapabilities.PHYJAM)
        else:
            raise ValueError('Parameter "method" must be either \'reflexive\' or \'constant\'.')

        self.handle.RF_promiscuity(1)
        self.handle.RF_autocrc(0)

        if channel is not None:
            self.set_channel(channel, page)

        self.handle.CC_RFST_RX()

        if method == "reflexive":
            self.handle.RF_reflexjam() 
        elif method == 'constant':
            self.handle.RF_carrier()  

    #TODO maybe move sync to byte string rather than int
    def set_sync(self, sync: int=0xA70F) -> Any:
        '''Set the register controlling the 802.15.4 PHY sync byte.'''
        if self.handle is None:
            raise Exception("Handle does not exist")

        self.capabilities.require(KBCapabilities.SET_SYNC)
        if (sync >> 16) > 0:
            raise Exception("Sync word (%x) must be 2-bytes or less." % sync)
        return self.handle.poke(CC2420_REG_SYNC, sync)

    def jammer_off(self) -> None:
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        #TODO implement
        raise Exception('Not yet implemented')


from typing import Optional, Dict, Union, Any, List

import os
import time
import struct
import time 
from datetime import datetime, timedelta 
from .kbutils import KBCapabilities, makeFCS 
from .GoodFETCCSPI import GoodFETCCSPI

class APIMOTE:
    def __init__(self, dev: str) -> None:

        self._channel: Optional[int] = None
        self._page: int = 0
        self.handle: Optional[Any] = None
        self.dev: str = dev

        self.handle = GoodFETCCSPI()
        self.handle.serInit(port=self.dev)
        self.handle.setup()

        self.__stream_open: bool = False

        self.capabilities: KBCapabilities = KBCapabilities()
        self.__set_capabilities()

    def __set_capabilities(self) -> None:
        self.capabilities.setcapab(KBCapabilities.NONE, False)

        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.FREQ_900, False)
        self.capabilities.setcapab(KBCapabilities.FREQ_863, False)
        self.capabilities.setcapab(KBCapabilities.FREQ_868, False)
        self.capabilities.setcapab(KBCapabilities.FREQ_870, False)
        self.capabilities.setcapab(KBCapabilities.FREQ_915, False)

        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        self.capabilities.setcapab(KBCapabilities.INJECT, True)
        self.capabilities.setcapab(KBCapabilities.SELFACK, False)
        self.capabilities.setcapab(KBCapabilities.PHYJAM, False)
        self.capabilities.setcapab(KBCapabilities.PHYJAM_REFLEX, False)
        self.capabilities.setcapab(KBCapabilities.SET_SYNC, False)
        return

    def close(self) -> None:
        
        if self.handle is None:
            raise Exception("Handle does not exist");

        self.handle = None

    def check_capability(self, capab: int) -> bool:
        return self.capabilities.check(capab)

    def get_capabilities(self) -> Dict[int, bool]:
        return self.capabilities.getlist()

    def get_dev_info(self) -> List[Union[str, Any]]:
        return [self.dev, "GoodFET Apimote", ""]

    def sniffer_on(self, channel: Optional[int]=None, page: int=0) -> None:
        self.capabilities.require(KBCapabilities.SNIFF)
        
        if self.handle is None:
            raise Exception("Handle does not exist")

        if channel is None:
            pass
        else:
            self.set_channel(channel, page)
        
        self.__stream_open = True

    def sniffer_off(self) -> None:
        self.__stream_open = False

    def set_channel(self, channel: int, page: int=0) -> None:
        self.capabilities.require(KBCapabilities.SETCHAN)

        if self.handle is None:
            raise Exception("Handle does not exist")

        if channel >= 11 and channel <= 26:
            self._channel = channel
        else:
            raise Exception('Invalid channel')

    def inject(self, packet: bytes, channel: Optional[int]=None, count: int=1, delay: int=0, page: int=0) -> None:
        self.capabilities.require(KBCapabilities.INJECT)

        if self.handle is None:
            raise Exception("Handle does not exist")


        if len(packet) < 1:
            raise Exception('Empty packet')
        if len(packet) > 125:                   # 127 - 2 to accommodate FCS
            raise Exception('Packet too long')

        if channel is None:
            pass
        else:
            self.set_channel(channel, page)

    def pnext(self, timeout: int=100) -> Any:
        self.capabilities.require(KBCapabilities.SNIFF)

        if self.handle is None:
            raise Exception("Handle does not exist")

        if self.__stream_open == False:
            self.sniffer_on()

        packet: Optional[bytes] = None
        frame: Optional[bytes] = None
        validcrc: bool = False
        rssi: Optional[int] = None

        if frame is None:
            pass
        else:
            if frame[-2:] == makeFCS(frame[:-2]):
                validcrc = True

        result: Dict[Union[int, str], Any] = {
            0: frame, 
            1: validcrc, 
            2: rssi, 
            'bytes': frame, 
            'validcrc': validcrc, 
            'rssi': rssi, 
            'location': None,
            'datetime': datetime.utcnow()
        }
        if rssi is None:
            result['dbm'] = None
        else:
            result['dbm'] = rssi - 45

        return result
 
    def ping(self, da: Any, panid: Any, sa: Any, channel: Optional[int]=None, page: int=0) -> None:
        raise Exception('Not yet implemented')

    def jammer_on(self, channel: Optional[int]=None, page: int=0) -> None:
        self.capabilities.require(KBCapabilities.PHYJAM_REFLEX)

        if self.handle is None:
            raise Exception("Handle does not exist")

        if channel is None:
            pass
        else:
            self.set_channel(channel, page)

    def set_sync(self, sync: int=0xA70F) -> Any:
        self.capabilities.require(KBCapabilities.SET_SYNC)

        if self.handle is None:
            raise Exception("Handle does not exist")

    def jammer_off(self, channel: Optional[int]=None, page: int=0) -> None:
        raise Exception('Not yet implemented')


from typing import Optional, Union, Tuple, Any, List

DEFAULT_KB_CHANNEL: int = 11
DEFAULT_KB_PAGE: Optional[int] = None
DEFAULT_KB_DEVICE: Optional[int] = None

from scapy.config import conf # type: ignore
setattr(conf, 'killerbee_channel', DEFAULT_KB_CHANNEL)
setattr(conf, 'killerbee_page', DEFAULT_KB_PAGE)
setattr(conf, 'killerbee_device', DEFAULT_KB_DEVICE)
setattr(conf, 'killerbee_nkey', None)
from scapy import plist # type: ignore
from scapy.base_classes import SetGen # type: ignore
from scapy.packet import Gen, Raw # type: ignore
from scapy.all import Dot15d4 # type: ignore
from scapy.all import Dot15d4FCS # type: ignore
from scapy.all import PcapWriter # type: ignore
from scapy.all import ZigbeeSecurityHeader # type: ignore
from scapy.all import ZigbeeNWK # type: ignore
from scapy.all import ZigbeeAppDataPayload # type: ignore
from scapy.all import ZigbeeNWKCommandPayload # type: ignore
from scapy.all import Packet # type: ignore

# This line will allow KillerBee's pcap reader to overwrite scapy's reader that is imported on the
# above line, per suggestion from cutaway at https://code.google.com/p/killerbee/issues/detail?id=28:

from killerbee import *

import os, time, struct
from .kbutils import randmac

import logging
log_killerbee = logging.getLogger('scapy.killerbee')

# new scapy needs to know if we're using sixlowpan or zigbee
conf.dot15d4_protocol = "zigbee"

def __kb_send(kb: KillerBee, x: Union[str, Gen], channel: Optional[int]=None, page: int=0, inter: int=0, loop: int=0, count: Optional[int]=None, verbose: Optional[int]=None, realtime: Optional[int]=None, *args: Any, **kargs: Any) -> int:
    if type(x) is str:
        x = Raw(load=x)
    if not isinstance(x, Gen):
        x = SetGen(x)
    if verbose is None:
        verbose = conf.verb

    n: int = 0
    if count is not None:
        loop = -count
    elif not loop:
        loop=-1

    dt0: Optional[int] = None
    try:
        while loop:
            for p in x:
                if realtime:
                    ct = time.time()
                    if dt0:
                        st = dt0+p.time-ct
                        if st > 0:
                            time.sleep(st)
                    else:
                        dt0 = ct-p.time
                kb.inject(p.do_build()[:-2], channel = None, count = 1, delay = 0, page = 0)  # [:-2] because the firmware adds the FCS
                n += 1
                if verbose:
                    os.write(1,b".")
                time.sleep(inter)
            if loop < 0:
                loop += 1
    except KeyboardInterrupt:
        pass
    return n

def __kb_recv(kb: KillerBee, count: int=0, store: int=1, prn: Optional[Any]=None, lfilter: Optional[Any]=None, stop_filter: Optional[Any]=None, verbose: Optional[int]=None, timeout: Optional[int]=None) -> List[bytes]:
    kb.sniffer_on()
    if timeout is not None:
        stoptime = time.time()+timeout
    if verbose is None:
        verbose = conf.verb

    lst: List[Any] = []
    packetcount: int = 0
    remain: Optional[float] = None
    while 1:
        try:
            if timeout is not None:
                remain = stoptime-time.time()
                if remain <= 0:
                    break

            packet: Any = kb.pnext() # int(remain * 1000) to convert to seconds
            if packet is None: continue
            if verbose > 1:
                os.write(1, b"*")
            packet = Dot15d4(packet[0])
            if lfilter and not lfilter(packet):
                continue
            packetcount += 1
            if store:
                lst.append(packet)
            if prn:
                r = prn(packet)
            if stop_filter and stop_filter(packet):
                break
            if count > 0 and packetcount >= count:
                break
        except KeyboardInterrupt:
            break

    kb.sniffer_off()
    kb.close()
    return lst

@conf.commands.register
def kbdev() -> None:
    """List KillerBee recognized devices"""
    show_dev()

@conf.commands.register
def kbsendp(pkt: bytes, channel: Optional[int]=None, inter: int=0, loop: int=0, iface: Optional[Any]=None, count: Optional[int]=None, verbose: Optional[int]=None, realtime: Optional[int]=None, page: int=0) -> None:
    """
    Send a packet with KillerBee
    @param channel:  802.15.4 channel to transmit/receive on
    @param page:     802.15.4 subghz page to transmit/receive on
    @param inter:    time to wait between tranmissions
    @param loop:     number of times to process the packet list
    @param iface:    KillerBee interface to use, or KillerBee() class instance
    @param verbose:  set verbosity level
    @param realtime: use packet's timestamp, bending time with realtime value
    """
    if channel is None:
        channel = conf.killerbee_channel
    if not page:
        page = conf.killerbee_page
    if not isinstance(iface, KillerBee):
        if iface is not None:
            kb: KillerBee = KillerBee(device = iface)
        else:
            kb = KillerBee(device = conf.killerbee_device)
        kb.set_channel(channel, page)
    else:
        kb = iface

    # Make sure the packet has 2 bytes for FCS before TX
    if not Dot15d4FCS in pkt:
        pkt/=Raw("\x00\x00")

    pkts_out: int = __kb_send(kb, pkt, inter = inter, loop = loop, count = count, verbose = verbose, realtime = realtime)
    if verbose:
        print("\nSent {} packets.".format(pkts_out))
    kb.close()

@conf.commands.register
def kbsrp(pkt: bytes, channel: Optional[int]=None, page: int=0, inter: int=0, count: int=0, iface: Optional[Any]=None, store: int=1, prn: Optional[Any]=None, lfilter: Optional[int]=None, timeout: Optional[int]=None, verbose: Optional[int]=None, realtime: Optional[int]=None) -> plist.PacketList:
    """
    Send and receive packets with KillerBee
    @param channel:  802.15.4 channel to transmit/receive on
    @param page:     802.15.4 subghz page to transmit/receive on
    @param inter:    time to wait between tranmissions
    @param count:    number of packets to capture. 0 means infinity
    @param iface:    KillerBee interface to use, or KillerBee() class instance
    @param store:    wether to store sniffed packets or discard them
    @param prn:      function to apply to each packet. If something is returned,
                      it is displayed. Ex:
                      ex: prn = lambda x: x.summary()
    @param lfilter:  python function applied to each packet to determine
                      if further action may be done
                      ex: lfilter = lambda x: x.haslayer(Padding)
    @param timeout:  stop sniffing after a given time (default: None)
    @param verbose:  set verbosity level
    @param realtime: use packet's timestamp, bending time with realtime value
    """
    if verbose is None:
        verbose = conf.verb
    if channel is None:
        channel = conf.killerbee_channel
    if not page:
        page = conf.killerbee_page
    if not isinstance(iface, KillerBee):
        if iface is not None:
            kb: KillerBee = KillerBee(device = iface)
        else:
            kb = KillerBee(device = conf.killerbee_device)
        kb.set_channel(channel, page)
    else:
        kb = iface

    # Make sure the packet has an FCS layer before TX
    if not Dot15d4FCS in pkt:
        pkt/=Raw("\x00\x00")

    pkts_out: int = __kb_send(kb, pkt, inter = inter, loop = 0, count = None, verbose = verbose, realtime = realtime)
    if verbose:
        print("\nSent %i packets." % pkts_out)

    pkts_in: List[bytes] = __kb_recv(kb, count = count, store = store, prn = prn, lfilter = lfilter, verbose = verbose, timeout = timeout)
    if verbose:
        print("\nReceived %i packets." % len(pkts_in))

    return plist.PacketList(pkts_in, 'Results')

@conf.commands.register
def kbsrp1(pkt: bytes, channel: Optional[int]=None, page: int=0, inter: int=0, iface: Optional[Any]=None, store: int=1, prn: Optional[Any]=None, lfilter: Optional[int]=None, timeout: Optional[int]=None, verbose: Optional[int]=None, realtime: Optional[int]=None) -> plist.PacketList:
    """Send and receive packets with KillerBee and return only the first answer"""
    return kbsrp(pkt, channel = channel, page = page, inter = inter, count = 1, iface = iface, store = store, prn = prn, lfilter = lfilter, timeout = timeout, verbose = verbose, realtime = realtime)

@conf.commands.register
def kbsniff(channel: Optional[int]=None, page: int=0, count: int=0, iface: Optional[Any]=None, store: int=1, prn: Optional[int]=None, lfilter: Optional[int]=None, stop_filter: Any=None, verbose: Optional[int]=None, timeout: Optional[int]=None) -> plist.PacketList:
    """
    Sniff packets with KillerBee.
    @param channel:  802.15.4 channel to transmit/receive on
    @param page:     802.15.4 subghz page to transmit/receive on
    @param count:    number of packets to capture. 0 means infinity
    @param iface:    KillerBee interface to use, or KillerBee() class instance
    @param store:    whether to store sniffed packets or discard them
    @param prn:      function to apply to each packet. If something is returned,
                      it is displayed. Ex:
                      ex: prn = lambda x: x.summary()
    @param lfilter:  python function applied to each packet to determine
                      if further action may be done
                      ex: lfilter = lambda x: x.haslayer(Padding)
    @param timeout:  stop sniffing after a given time (default: None)
    """
    if channel is None:
        channel = conf.killerbee_channel
    if not page:
        page = conf.killerbee_page
    if not isinstance(iface, KillerBee):
        if iface is not None:
            kb: KillerBee = KillerBee(device = iface)
        else:
            kb = KillerBee(device = conf.killerbee_device)
        kb.set_channel(channel, page)
    else:
        kb = iface

    return plist.PacketList(__kb_recv(kb, count = count, store = store, prn = prn, lfilter = lfilter, stop_filter = stop_filter, verbose = verbose, timeout = timeout), 'Sniffed')

@conf.commands.register
def kbrdpcap(filename: str, count: int=-1, skip: int=0, nofcs: bool=False) -> plist.PacketList:
    """
    Read a pcap file with the KillerBee library.
    Wraps the PcapReader to return scapy packet object from pcap files.
    This uses the killerbee internal methods instead of the scapy native methods.
    This is not necessarily better, and suggestions are welcome.
    Specify nofcs parameter as True if for some reason the packets in the PCAP
    don't have FCS (checksums) at the end.
    @return: Scapy packetlist of Dot15d4 packets parsed from the given PCAP file.
    """
    cap: PcapReader = PcapReader(filename)
    lst: List[bytes] = []
    packetcount: int = 0
    if count > 0:
        count += skip

    while 1:
        packet: bytes = cap.pnext()
        packetcount += 1
        if packet[1] == None:
            break
        if skip > 0 and packetcount <= skip:
            continue
        if nofcs: packet = Dot15d4(packet[1])
        else:     packet = Dot15d4FCS(packet[1])
        lst.append(packet)
        if count > 0 and packetcount >= count:
            break

    return plist.PacketList(lst, os.path.basename(filename))

@conf.commands.register
def kbwrpcap(save_file: str, pkts: List[bytes]) -> None:
    """
    Write a pcap using the KillerBee library.
    """
    pd: PcapWriter = PcapWriter(save_file)
    for packet in pkts:
        pd.write(bytes(packet))
    pd.close()

@conf.commands.register
def kbrddain(filename: str, count: int=-1, skip: int=0) -> plist.PacketList:
    """
    Read a daintree file with the KillerBee library
    Wraps the DainTreeReader to return scapy packet object from daintree files.
    """
    cap: DainTreeReader = DainTreeReader(filename)
    lst: List[Any] = []
    packetcount: int = 0

    while 1:
        packet: Optional[List[Any]] = cap.pnext()
        if packet is None:
          break;
        packetcount += 1
        if packet[1] is None:
            break
        if skip > 0 and packetcount <= skip:
            continue
        packet = Dot15d4(packet[1])
        lst.append(packet)
        if count > 0  and packetcount >= count:
            break
    return plist.PacketList(lst, os.path.basename(filename))

@conf.commands.register
def kbwrdain(save_file: str, pkts: List[Any]) -> None:
    """
    Write a daintree file using the KillerBee library.
    """
    dt: DainTreeDumper = DainTreeDumper(save_file)
    for packet in pkts:
        dt.pwrite(bytes(packet))
    dt.close()

@conf.commands.register
def kbkeysearch(packet: Any, searchdata: str, ispath: bool=True, skipfcs: bool=True, raw: bool=False) -> Optional[str]:
    """
    Search a binary file for the encryption key to an encrypted packet.
    """
    if 'fcf_security' in packet.fields and packet.fcf_security == 0:
        raise Exception('Packet Not Encrypted (fcf_security Not Set)')
    if ispath:
        searchdata = open(searchdata, 'r').read()
    packet = packet.do_build()
    if skipfcs:
        packet = packet[:-2]
    offset: int = 0
    keybytes: List[bytes] = []
    d: Dot154PacketParser = Dot154PacketParser()
    searchdatalen: int = len(searchdata)
    while (offset < (searchdatalen - 16)):
        if d.decrypt(packet, searchdata[offset:offset+16]) != '':
            if raw:
                return ''.join(searchdata[offset + i] for i in range(0, 16))
            else:
                return ':'.join("%02x" % ord(searchdata[offset + i]) for i in range(0, 16))
        else:
            offset+=1
    return None

@conf.commands.register
def kbgetnetworkkey(pkts: Gen) -> Dict[str, str]:
    """
    Search packets for a plaintext key exchange returns the first one found.
    """
    if not isinstance(pkts, Gen):
        pkts = SetGen(pkts)
    for packet in pkts:
        packet = bytes(packet)
        zmac = Dot154PacketParser()
        znwk = ZigBeeNWKPacketParser()
        zaps = ZigBeeAPSPacketParser()
        try:
            # Process MAC layer details
            zmacpayload = zmac.pktchop(packet)[-1]
            if zmacpayload == None:
                continue

            # Process NWK layer details
            znwkpayload = znwk.pktchop(zmacpayload)[-1]
            if znwkpayload == None:
                continue

            # Process the APS layer details
            zapschop = zaps.pktchop(znwkpayload)
            if zapschop == None:
                continue

            # See if this is an APS Command frame
            apsfc = ord(zapschop[0])
            if (apsfc & ZBEE_APS_FCF_FRAME_TYPE) != ZBEE_APS_FCF_CMD:
                continue

            # Delivery mode is Normal Delivery (0)
            apsdeliverymode = (apsfc & ZBEE_APS_FCF_DELIVERY_MODE) >> 2
            if apsdeliverymode != 0:
                continue

            # Ensure Security is Disabled
            if (apsfc & ZBEE_APS_FCF_SECURITY) == 1:
                continue

            zapspayload = zapschop[-1]

            # Check payload length, must be at least 35 bytes
            # APS cmd | key type | key | sequence number | dest addr | src addr
            if len(zapspayload) < 35:
                continue

            # Check for APS command identifier Transport Key (0x05)
            if ord(zapspayload[0]) != 5:
                continue

            # Transport Key Frame, get the key type.  Network Key is 0x01, no
            # other keys should be sent in plaintext
            if ord(zapspayload[1]) != 1:
                continue

            # Reverse these fields
            networkkey = zapspayload[2:18][::-1]
            destaddr = zapspayload[19:27][::-1]
            srcaddr = zapspayload[27:35][::-1]

            key: Dict[str, str] = {}
            key['key'] = ':'.join("%02x" % ord(networkkey[x]) for x in range(16))
            key['dst'] = ':'.join("%02x" % ord(destaddr[x]) for x in range(8))
            key['src'] = ':'.join("%02x" % ord(srcaddr[x]) for x in range(8))
            return key
        except:
            continue
    return { }

@conf.commands.register
def kbtshark(store: int=0, *args: Any, **kargs: Any) -> bytes:
    """Sniff packets using KillerBee and print them calling pkt.show()"""
    return kbsniff(prn=lambda x: x.display(), store = store, *args, **kargs)

@conf.commands.register
def kbrandmac(length: int=8) -> str:
    """Returns a random MAC address using a list valid OUI's from ZigBee device manufacturers."""
    return randmac(length)

@conf.commands.register
def kbdecrypt(source_pkt: Gen, key: Optional[Union[Dict[str, str], Any]]=None, verbose: Optional[int]=None, doMicCheck: bool =False) -> Optional[Tuple[bytes, bool]]:
    """Decrypt Zigbee frames using AES CCM* with 32-bit MIC"""
    if verbose is None:
        verbose = conf.verb
    if key is None:
        if conf.killerbee_nkey == None:
            log_killerbee.error("Cannot find decryption key. (Set conf.killerbee_nkey)")
            return None
        key = conf.killerbee_nkey
    if len(key) != 16:
        log_killerbee.error("Invalid decryption key, must be a 16 byte string.")
        return None
    if not ZigbeeSecurityHeader in source_pkt:
        log_killerbee.error("Cannot decrypt frame without a ZigbeeSecurityHeader.")
        return None
    if not ZigbeeNWK in source_pkt:
        log_killerbee.error("Cannot decrypt frame without a ZigbeeNWK.")
        return None
    try:
        import zigbee_crypt # type: ignore
    except ImportError:
        log_killerbee.error("Could not import zigbee_crypt extension, cryptographic functionality is not available.")
        return None

    # This function destroys the packet, therefore work on a copy - @cutaway
    pkt: Gen = source_pkt.copy()

    # DOT154_CRYPT_ENC_MIC32 is always used regardless of what is claimed in OTA packet, so we will force it here.
    # This is done because the value of nwk_seclevel in the ZigbeeSecurityHeader does
    # not have to be accurate in the transmitted frame: the Zigbee NWK standard states that
    # the nwk_seclevel should be overwritten in the received frame with the value that is being
    # used by all nodes in the Zigbee network - this is to ensure that unencrypted frames can't be
    # maliciously injected.  i.e. the receiver shouldn't trust the received nwk_seclevel.
    # Recreate 'pkt' by rebuilding the raw data and mic to match:
    pkt.nwk_seclevel = DOT154_CRYPT_ENC_MIC32
    pkt.data += pkt.mic
    pkt.mic = pkt.data[-4:]
    pkt.data = pkt.data[:-4]

    encrypted: bytes = pkt.data
    # So calculate an amount to crop, equal to the size of the encrypted data and mic.  Note that
    # if there was an FCS, scapy will have already stripped it, so it will not returned by the
    # do_build() call below (and hence doesn't need to be taken into account in crop_size).
    crop_size: int = len(pkt.mic) + len(pkt.data)

    # create NONCE (for crypt) and zigbeeData (for MIC) according to packet type
    sec_ctrl_byte = bytes(pkt[ZigbeeSecurityHeader])[0:1]
    if ZigbeeAppDataPayload in pkt:
        nonce: bytes = struct.pack('L',source_pkt[ZigbeeNWK].ext_src)+struct.pack('I',source_pkt[ZigbeeSecurityHeader].fc) + sec_ctrl_byte
        zigbeeData: bytes = pkt[ZigbeeAppDataPayload].do_build()
    else:
        nonce = struct.pack('L',source_pkt[ZigbeeSecurityHeader].source)+struct.pack('I',source_pkt[ZigbeeSecurityHeader].fc) + sec_ctrl_byte
        zigbeeData = pkt[ZigbeeNWK].do_build()
    # For zigbeeData, we need the entire zigbee packet, minus the encrypted data and mic (4 bytes).
    zigbeeData = zigbeeData[:-crop_size]

    (payload, micCheck) = zigbee_crypt.decrypt_ccm(key, nonce, pkt.mic, encrypted, zigbeeData)

    if verbose > 2:
        print("Decrypt Details:")
        print("\tKey:            {!r}".format(key))
        print("\tNonce:          {!r}".format(nonce))
        print("\tZigbeeData:     {!r}".format(zigbeeData))
        print("\tEncrypted Data: {!r}".format(encrypted))
        print("\tDecrypted Data: {}".format(payload))
        print("\tMic:            {}".format(pkt.mic))

    frametype = pkt[ZigbeeNWK].frametype
    if frametype == 0 and micCheck == 1:
        payload = ZigbeeAppDataPayload(payload)
    elif frametype == 1 and micCheck == 1:
        payload = ZigbeeNWKCommandPayload(payload)
    else:
        payload = Raw(payload)

    if doMicCheck == False:
        return payload
    else:
        if micCheck == 1: return (payload, True)
        else:             return (payload, False)

@conf.commands.register
def kbencrypt(source_pkt: Gen, data: bytes, key: Optional[bytes]=None, verbose: Optional[int]=None) -> Optional[Gen]:
    """Encrypt Zigbee frames using AES CCM* with 32-bit MIC"""
    if verbose is None:
        verbose = conf.verb
    if key is None:
        if conf.killerbee_nkey == None:
            log_killerbee.error("Cannot find decryption key. (Set conf.killerbee_nkey)")
            return None
        key = conf.killerbee_nkey
    if len(key) != 16:
        log_killerbee.error("Invalid encryption key, must be a 16 byte string.")
        return None
    if not ZigbeeSecurityHeader in source_pkt:
        log_killerbee.error("Cannot encrypt frame without a ZigbeeSecurityHeader.")
        return None
    if not ZigbeeNWK in source_pkt:
        log_killerbee.error("Cannot encrypt frame without a ZigbeeNWK.")
        return None
    try:
        import zigbee_crypt
    except ImportError:
        log_killerbee.error("Could not import zigbee_crypt extension, cryptographic functionality is not available.")
        return None

    # This function destroys the packet, therefore work on a copy - @cutaway
    pkt = source_pkt.copy()

    # DOT154_CRYPT_ENC_MIC32 is always used regardless of what is claimed in OTA packet, so we will force it here.
    # This is done because the value of nwk_seclevel in the ZigbeeSecurityHeader does
    # not have to be accurate in the transmitted frame: the Zigbee NWK standard states that
    # the nwk_seclevel should be overwritten in the received frame with the value that is being
    # used by all nodes in the Zigbee network - this is to ensure that unencrypted frames can't be
    # maliciously injected.  i.e. the receiver shouldn't trust the received nwk_seclevel.
    pkt.nwk_seclevel = DOT154_CRYPT_ENC_MIC32

    # clear data and mic as we are about to create them
    pkt.data = ''
    pkt.mic = ''

    if isinstance(data, Packet):
        decrypted: bytes = data.do_build()
    else:
        decrypted = data

    # create NONCE (for crypt) and zigbeeData (for MIC) according to packet type
    sec_ctrl_byte = bytes(pkt[ZigbeeSecurityHeader])[0:1]
    if ZigbeeAppDataPayload in pkt:
        nonce = struct.pack('L',source_pkt[ZigbeeNWK].ext_src)+struct.pack('I',source_pkt[ZigbeeSecurityHeader].fc) + sec_ctrl_byte
        zigbeeData: bytes = pkt[ZigbeeAppDataPayload].do_build()
    else:
        nonce = struct.pack('L',source_pkt[ZigbeeSecurityHeader].source)+struct.pack('I',source_pkt[ZigbeeSecurityHeader].fc) + sec_ctrl_byte
        zigbeeData = pkt[ZigbeeNWK].do_build()

    # minimum security level is DOT154_CRYPT_ENC_MIC32 but provide more if requested
    miclen= kbgetmiclen(source_pkt.nwk_seclevel)
    if miclen < 4:
        miclen= 4

    (payload, mic) = zigbee_crypt.encrypt_ccm(key, nonce, miclen, decrypted, zigbeeData)

    if verbose > 2:
        print("Encrypt Details:")
        print("\tKey:            {!r}".format(key))
        print("\tNonce:          {!r}".format(nonce))
        print("\tZigbeeData:     {!r}".format(zigbeeData))
        print("\tDecrypted Data: {!r}".format(decrypted))
        print("\tEncrypted Data: {}".format(payload))
        print("\tMic:            {}".format(mic))

    # According to comments in e.g. https://github.com/wireshark/wireshark/blob/master/epan/dissectors/packet-zbee-security.c nwk_seclevel is not used any more but
    # we should reconstruct and return what was asked for anyway.
    pkt.data = payload + mic
    pkt.nwk_seclevel = source_pkt.nwk_seclevel
    ota_miclen= kbgetmiclen(pkt.nwk_seclevel)
    if ota_miclen > 0:
        pkt.mic = pkt.data[-ota_miclen:]
        pkt.data = pkt.data[:-ota_miclen]
    return pkt

@conf.commands.register
def kbgetmiclen(seclevel: int) -> int:
    """Returns the MIC length in bytes for the specified packet security level"""
    lengths: Dict[int, int] = {DOT154_CRYPT_NONE:0, DOT154_CRYPT_MIC32:4, DOT154_CRYPT_MIC64:8, DOT154_CRYPT_MIC128:16, DOT154_CRYPT_ENC:0, DOT154_CRYPT_ENC_MIC32:4, DOT154_CRYPT_ENC_MIC64:8, DOT154_CRYPT_ENC_MIC128:16}

    return lengths[seclevel]

@conf.commands.register
def kbgetpanid(packet: Gen) -> Tuple[Optional[bytes], Optional[Any]]:
    """Returns the pan id and which layer it was found in or None, None"""
    for layer in packet.layers():
        for field in packet[layer].fields:
            if 'dest_panid' in field:
                return packet[layer].dest_panid, layer
    return None, None

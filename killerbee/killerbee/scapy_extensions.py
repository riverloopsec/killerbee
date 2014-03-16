DEFAULT_KB_CHANNEL = 11
DEFAULT_KB_DEVICE = None

from scapy.config import conf
setattr(conf, 'killerbee_channel', DEFAULT_KB_CHANNEL)
setattr(conf, 'killerbee_device', DEFAULT_KB_DEVICE)
setattr(conf, 'killerbee_nkey', None)
from scapy.base_classes import SetGen
from scapy.packet import Gen, Raw
from scapy.all import *

import os, time, struct
from kbutils import randmac

import logging
log_killerbee = logging.getLogger('scapy.killerbee')

def __kb_send(kb, x, channel = None, inter = 0, loop = 0, count = None, verbose = None, realtime = None, *args, **kargs):
    if type(x) is str:
        x = Raw(load=x)
    if not isinstance(x, Gen):
        x = SetGen(x)
    if verbose is None:
        verbose = conf.verb

    n = 0
    if count is not None:
        loop = -count
    elif not loop:
        loop=-1
    dt0 = None
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
                kb.inject(p.do_build()[:-2], channel = None, count = 1, delay = 0)  # [:-2] because the firmware adds the FCS
                n += 1
                if verbose:
                    os.write(1,".")
                time.sleep(inter)
            if loop < 0:
                loop += 1
    except KeyboardInterrupt:
        pass
    return n

def __kb_recv(kb, count = 0, store = 1, prn = None, lfilter = None, stop_filter = None, verbose = None, timeout = None):
    kb.sniffer_on()
    if timeout is not None:
        stoptime = time.time()+timeout
    if verbose is None:
        verbose = conf.verb

    lst = []
    packetcount = 0
    remain = None
    while 1:
        try:
            if timeout is not None:
                remain = stoptime-time.time()
                if remain <= 0:
                    break

            packet = kb.pnext() # int(remain * 1000) to convert to seconds
            if packet == None: continue
            if verbose > 1:
                os.write(1, "*")
            packet = Dot15d4(packet[0])
            if lfilter and not lfilter(packet):
                continue
            packetcount += 1
            if store:
                lst.append(packet)
            if prn:
                r = prn(packet)
                if r is not None:
                    print r
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
def kbdev():
    """List KillerBee recognized devices"""
    show_dev()

@conf.commands.register
def kbsendp(pkt, channel = None, inter = 0, loop = 0, iface = None, verbose = None, realtime=None):
    """
    Send a packet with KillerBee
    @param channel:  802.15.4 channel to transmit/receive on
    @param inter:    time to wait between tranmissions
    @param loop:     number of times to process the packet list
    @param iface:    KillerBee interface to use, or KillerBee() class instance
    @param verbose:  set verbosity level
    @param realtime: use packet's timestamp, bending time with realtime value
    """
    if channel == None:
        channel = conf.killerbee_channel
    if not isinstance(iface, KillerBee):
        if iface is not None:
            kb = KillerBee(device = iface)
        else:
            kb = KillerBee(device = conf.killerbee_device)
        kb.set_channel(channel)
    else:
        kb = iface
    
    pkts_out = __kb_send(kb, pkt, inter = inter, loop = loop, count = None, verbose = verbose, realtime = realtime)
    print "\nSent %i packets." % pkts_out

@conf.commands.register
def kbsrp(pkt, channel = None, inter = 0, count = 0, iface = None, store = 1, prn = None, lfilter = None, timeout = None, verbose = None, realtime = None):
    """
    Send and receive packets with KillerBee
    @param channel:  802.15.4 channel to transmit/receive on
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
    if channel == None:
        channel = conf.killerbee_channel
    if not isinstance(iface, KillerBee):
        if iface is not None:
            kb = KillerBee(device = iface)
        else:
            kb = KillerBee(device = conf.killerbee_device)
        kb.set_channel(channel)
    else:
        kb = iface
    
    pkts_out = __kb_send(kb, pkt, inter = inter, loop = 0, count = None, verbose = verbose, realtime = realtime)
    if verbose:
        print "\nSent %i packets." % pkts_out

    pkts_in = __kb_recv(kb, count = count, store = store, prn = prn, lfilter = lfilter, verbose = verbose, timeout = timeout)
    if verbose:
        print "\nReceived %i packets." % len(pkts_in)
    return plist.PacketList(pkts_in, 'Results')

@conf.commands.register
def kbsrp1(pkt, channel = None, inter = 0, iface = None, store = 1, prn = None, lfilter = None, timeout = None, verbose = None, realtime = None):
    """Send and receive packets with KillerBee and return only the first answer"""
    return kbsrp(pkt, channel = channel, inter = inter, count = 1, iface = iface, store = store, prn = prn, lfilter = lfilter, timeout = timeout, verbose = verbose, realtime = realtime)

@conf.commands.register 
def kbsniff(channel = None, count = 0, iface = None, store = 1, prn = None, lfilter = None, stop_filter = None, verbose = None, timeout = None):
    """
    Sniff packets with KillerBee.
    @param channel:  802.15.4 channel to transmit/receive on
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
    """
    if channel == None:
        channel = conf.killerbee_channel
    if not isinstance(iface, KillerBee):
        if iface is not None:
            kb = KillerBee(device = iface)
        else:
            kb = KillerBee(device = conf.killerbee_device)
        kb.set_channel(channel)
    else:
        kb = iface
    return plist.PacketList(__kb_recv(kb, count = count, store = store, prn = prn, lfilter = lfilter, stop_filter = stop_filter, verbose = verbose, timeout = timeout), 'Sniffed')

@conf.commands.register
def kbrdpcap(filename, count = -1, skip = 0, nofcs=False):
    """
    Read a pcap file with the KillerBee library.
    Wraps the PcapReader to return scapy packet object from pcap files.
    This uses the killerbee internal methods instead of the scapy native methods.
    This is not necessarily better, and suggestions are welcome.
    Specify nofcs parameter as True if for some reason the packets in the PCAP
    don't have FCS (checksums) at the end.
    @return: Scapy packetlist of Dot15d4 packets parsed from the given PCAP file.
    """
    cap = PcapReader(filename)
    lst = []
    packetcount = 0
    if count > 0:
        count += skip

    while 1:
        packet = cap.pnext()
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
def kbwrpcap(save_file, pkts):
    """
    Write a pcap using the KillerBee library.
    """
    pd = PcapDumper(DLT_IEEE802_15_4, save_file, ppi=False)
    for packet in pkts:
        pd.pcap_dump(str(packet))
    pd.close()

@conf.commands.register
def kbrddain(filename, count = -1, skip = 0):
    """
    Read a dain tree file with the KillerBee library
    Wraps the DainTreeReader to return scapy packet object from daintree files.
    """
    cap = DainTreeReader(filename)
    lst = []
    packetcount = 0

    while 1:
        packet = cap.pnext()
        packetcount += 1
        if packet[1] == None:
            break
        if skip > 0 and packetcount <= skip:
            continue
        packet = Dot15d4(packet[1])
        lst.append(packet)
        if count > 0 and packetcount >= count:
            break
    return plist.PacketList(lst, os.path.basename(filename))

@conf.commands.register
def kbwrdain(save_file, pkts):
    """
    Write a daintree file using the KillerBee library.
    """
    dt = DainTreeDumper(save_file)
    for packet in pkts:
        dt.pwrite(str(packet))
    dt.close()

@conf.commands.register
def kbkeysearch(packet, searchdata, ispath = True, skipfcs = True, raw = False):
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
    offset = 0
    keybytes = []
    d = Dot154PacketParser()
    searchdatalen = len(searchdata)
    while (offset < (searchdatalen - 16)):
        if d.decrypt(packet, searchdata[offset:offset+16]) != '':
            if raw:
                return ''.join(searchdata[offset + i] for i in xrange(0, 16))
            else:
                return ':'.join("%02x" % ord(searchdata[offset + i]) for i in xrange(0, 16))
        else:
            offset+=1
    return None

@conf.commands.register
def kbgetnetworkkey(pkts):
    """
    Search packets for a plaintext key exchange returns the first one found.
    """
    if not isinstance(pkts, Gen):
        pkts = SetGen(pkts)
    for packet in pkts:
        packet = str(packet)
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

            key_bytes = []
            dst_mac_bytes = []
            src_mac_bytes = []
            key = {}
            key['key'] = ':'.join("%02x" % ord(networkkey[x]) for x in xrange(16))
            key['dst'] = ':'.join("%02x" % ord(destaddr[x]) for x in xrange(8))
            key['src'] = ':'.join("%02x" % ord(srcaddr[x]) for x in xrange(8))
            return key
        except:
            continue
    return { }

@conf.commands.register
def kbtshark(store = 0, *args,**kargs):
    """Sniff packets using KillerBee and print them calling pkt.show()"""
    return kbsniff(prn=lambda x: x.display(), store = store, *args, **kargs)

@conf.commands.register
def kbrandmac(length = 8):
    """Returns a random MAC address using a list valid OUI's from ZigBee device manufacturers."""
    return randmac(length)

@conf.commands.register
def kbdecrypt(pkt, key = None, verbose = None):
    """Decrypt Zigbee frames using AES CCM* with 32-bit MIC"""
    if verbose is None:
        verbose = conf.verb
    if key == None:
        if conf.killerbee_nkey == None:
            log_killerbee.error("Cannot find decryption key. (Set conf.killerbee_nkey)")
            return None
        key = conf.killerbee_nkey
    if len(key) != 16:
        log_killerbee.error("Invalid decryption key, must be a 16 byte string.")
        return None
    elif not pkt.haslayer(ZigbeeSecurityHeader) or not pkt.haslayer(ZigbeeNWK):
        log_killerbee.error("Cannot decrypt frame without a ZigbeeSecurityHeader.")
        return None
    try:
        import zigbee_crypt
    except ImportError:
        log_killerbee.error("Could not import zigbee_crypt extension, cryptographic functionality is not available.")
        return None

    pkt.nwk_seclevel=5
    #mic = struct.unpack(">I", f['mic'])
    mic = pkt.mic

    f = pkt.getlayer(ZigbeeSecurityHeader).fields
    encrypted = f['data']

    sec_ctrl_byte = str(pkt.getlayer(ZigbeeSecurityHeader))[0]
    nonce = struct.pack('L',f['ext_source'])+struct.pack('I',f['fc']) + sec_ctrl_byte
    #nonce = "" # build the nonce
    #nonce += struct.pack(">Q", f['ext_source'])
    #nonce += struct.pack(">I", f['fc'])
    #fc = (f['reserved1'] << 6) | (f['extended_nonce'] << 5) | (f['key_type'] << 3) | f['reserved2']
    #nonce += chr(fc | 0x05)

    if verbose > 2:
        print "Decrypt Details:"
        print "\tKey:            " + key.encode('hex')
        print "\tNonce:          " + nonce.encode('hex')
        print "\tMic:            " + mic.encode('hex')
        print "\tEncrypted Data: " + encrypted.encode('hex')
    
    crop_size = 4 + 2 + len(pkt.getlayer(ZigbeeSecurityHeader).fields['data'])  # the size of all the zigbee crap, minus the length of the encrypted data, mic and FCS

    # the Security Control Field flags have to be adjusted before this is calculated, so we store their original values so we can reset them later
    #reserved2 = pkt.getlayer(ZigbeeSecurityHeader).fields['reserved2']
    #pkt.getlayer(ZigbeeSecurityHeader).fields['reserved2'] = (pkt.getlayer(ZigbeeSecurityHeader).fields['reserved2'] | 0x05)
    zigbeeData = pkt.getlayer(ZigbeeNWK).do_build()
    zigbeeData = zigbeeData[:-crop_size]
    #pkt.getlayer(ZigbeeSecurityHeader).fields['reserved2'] = reserved2
    
    (payload, micCheck) = zigbee_crypt.decrypt_ccm(key, nonce, mic, encrypted, zigbeeData)

    if verbose > 2:
        print "\tDecrypted Data: " + payload.encode('hex')

    frametype = pkt.getlayer(ZigbeeNWK).fields['frametype']
    if frametype == 0:
        payload = ZigbeeAppDataPayload(payload)
    elif frametype == 1:
        payload = ZigbeeNWKCommandPayload(payload)
    else:
        payload = Raw(payload)

    return payload
    #if micCheck == 1: return (payload, True)
    #else:             return (payload, False)

@conf.commands.register
def kbencrypt(pkt, data, key = None, verbose = None):
    """Encrypt Zigbee frames using AES CCM* with 32-bit MIC"""
    if verbose is None:
        verbose = conf.verb
    if key == None:
        if conf.killerbee_nkey == None:
            log_killerbee.error("Cannot find decryption key. (Set conf.killerbee_nkey)")
            return None
        key = conf.killerbee_nkey
    if len(key) != 16:
        log_killerbee.error("Invalid encryption key, must be a 16 byte string.")
        return None
    elif not pkt.haslayer(ZigbeeSecurityHeader) or not pkt.haslayer(ZigbeeNWK):
        log_killerbee.error("Cannot encrypt frame without a ZigbeeSecurityHeader.")
        return None
    try:
        import zigbee_crypt
    except ImportError:
        log_killerbee.error("Could not import zigbee_crypt extension, cryptographic functionality is not available.")
        return None

    f = pkt.getlayer(ZigbeeSecurityHeader).fields
    f['data'] = ''  # explicitly clear it out, this should go without say
    
    if isinstance(data, Packet):
        decrypted = data.do_build()
    else:
        decrypted = data

    nonce = ""  # build the nonce
    nonce += struct.pack(">Q", f['source'])
    nonce += struct.pack(">I", f['fc'])
    fc = (f['reserved1'] << 6) | (f['extended_nonce'] << 5) | (f['key_type'] << 3) | f['reserved2']
    nonce += chr(fc | 0x05)

    if verbose > 2:
        print "Encrypt Details:"
        print "\tKey:            " + key.encode('hex')
        print "\tNonce:          " + nonce.encode('hex')
        print "\tDecrypted Data: " + decrypted.encode('hex')
        
    crop_size = 4 + 2 # the size of all the zigbee crap, minus the length of the mic and FCS
    
    # the Security Control Field flags have to be adjusted before this is calculated, so we store their original values so we can reset them later
    reserved2 = pkt.getlayer(ZigbeeSecurityHeader).fields['reserved2']
    pkt.getlayer(ZigbeeSecurityHeader).fields['reserved2'] = (pkt.getlayer(ZigbeeSecurityHeader).fields['reserved2'] | 0x05)
    zigbeeData = pkt.getlayer(ZigbeeNWK).do_build()
    zigbeeData = zigbeeData[:-crop_size]
    pkt.getlayer(ZigbeeSecurityHeader).fields['reserved2'] = reserved2
    
    (payload, mic) = zigbee_crypt.encrypt_ccm(key, nonce, 4, decrypted, zigbeeData)

    if verbose > 2:
        print "\tEncrypted Data: " + payload.encode('hex')
        print "\tMic:            " + mic.encode('hex')
    
    # Set pkt's values to reflect the encrypted ones to it's ready to be sent
    f['data'] = payload
    f['mic'] = struct.unpack(">I", mic)[0]
    return pkt


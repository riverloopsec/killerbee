# KillerBee Device Support for:
# Wislab Sniffer Radio Client
# This sniffer is a remote IPv4 host.
# 
# (C) 2013 Ryan Speers <ryan at riverloopsecurity.com>
#
# For documentation from the vendor, visit:
#   http://www.sniffer.wislab.cz/sniffer-configuration/
#

import os
import time
import struct
import time
import urllib2
import re
from socket import socket, AF_INET, SOCK_DGRAM, SOL_SOCKET, SO_REUSEADDR, timeout as error_timeout
from struct import unpack

from datetime import datetime, date, timedelta
from kbutils import KBCapabilities, makeFCS, isIpAddr, KBInterfaceError

DEFAULT_IP = "10.10.10.2"   #IP address of the sniffer
DEFAULT_GW = "10.10.10.1"   #IP address of the default gateway
DEFAULT_UDP = 17754         #"Remote UDP Port"
TESTED_FW_VERS = ["0.5"]    #Firmware versions tested with the current version of this client device connector

NTP_DELTA = 70*365*24*60*60 #datetime(1970, 1, 1, 0, 0, 0) - datetime(1900, 1, 1, 0, 0, 0)

'''
Convert the two parts of an NTP timestamp to a datetime object.
Similar code from Wireshark source:
575 	/* NTP_BASETIME is in fact epoch - ntp_start_time */
576 	#define NTP_BASETIME 2208988800ul
619 	void
620 	ntp_to_nstime(tvbuff_t *tvb, gint offset, nstime_t *nstime)
621 	{
622 	nstime->secs = tvb_get_ntohl(tvb, offset);
623 	if (nstime->secs)
624 	nstime->secs -= NTP_BASETIME;
625 	nstime->nsecs = (int)(tvb_get_ntohl(tvb, offset+4)/(NTP_FLOAT_DENOM/1000000000.0));
626 	}
'''
def ntp_to_system_time(secs, msecs):
    """convert a NTP time to system time"""
    print "Secs:", secs, msecs
    print "\tUTC:", datetime.utcfromtimestamp(secs - 2208988800)
    return datetime.utcfromtimestamp(secs - 2208988800)

def getFirmwareVersion(ip):
    try:
        html = urllib2.urlopen("http://{0}/".format(ip))
        fw = re.search(r'Firmware version ([0-9.]+)', html.read())
        if fw is not None:
            return fw.group(1)
    except Exception as e:
        print("Unable to connect to IP {0} (error: {1}).".format(ip, e))
    return None

def getMacAddr(ip):
    '''
    Returns a string for the MAC address of the sniffer.
    '''
    try:
        html = urllib2.urlopen("http://{0}/".format(ip))
        # Yup, we're going to have to steal the status out of a JavaScript variable
        #var values = removeSSItag('<!--#pindex-->STOPPED,00:1a:b6:00:0a:a4,...
        res = re.search(r'<!--#pindex-->[A-Z]+,((?:[0-9a-f]{2}:){5}[0-9a-f]{2})', html.read())
        if res is None:
            raise KBInterfaceError("Unable to parse the sniffer's MAC address.")
        return res.group(1)
    except Exception as e:
        print("Unable to connect to IP {0} (error: {1}).".format(ip, e))
    return None

def isWislab(dev):
    return ( isIpAddr(dev) and getFirmwareVersion(dev) != None )

class WISLAB:
    def __init__(self, dev=DEFAULT_IP, recvport=DEFAULT_UDP, recvip=DEFAULT_GW):
        '''
        Instantiates the KillerBee class for the Wislab Sniffer.
        @type dev:   String
        @param dev:  IP address (ex 10.10.10.2)
        @type recvport: Integer
        @param recvport: UDP port to listen for sniffed packets on.
        @type recvip: String
        @param recvip: IP address of the host, where the sniffer will send sniffed packets to.
        @return: None
        @rtype: None
        '''
        self._channel = None
        self._modulation = 0 #unknown, will be set by change channel currently
        self.handle = None
        self.dev = dev
        
        #TODO The receive port and receive IP address are currently not 
        # obtained from or verified against the Wislab sniffer, nor are they
        # used to change the settings on the sniffer.
        self.udp_recv_port = recvport
        self.udp_recv_ip   = recvip

        self.__revision_num = getFirmwareVersion(self.dev)
        if self.__revision_num not in TESTED_FW_VERS:
            print("Warning: Firmware revision {0} reported by the sniffer is not currently supported. Errors may occur and dev_wislab.py may need updating.".format(self.__revision_num))

        self.handle = socket(AF_INET, SOCK_DGRAM)
        self.handle.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.handle.bind((self.udp_recv_ip, self.udp_recv_port))

        self.__stream_open = False
        self.capabilities = KBCapabilities()
        self.__set_capabilities()
        
    def close(self):
        '''Actually close the receiving UDP socket.'''
        self.sniffer_off()  # turn sniffer off if it's currently running
        self.handle.close() # socket.close()
        self.handle = None

    def check_capability(self, capab):
        return self.capabilities.check(capab)
    def get_capabilities(self):
        return self.capabilities.getlist()
    def __set_capabilities(self):
        '''
        Sets the capability information appropriate for the client and firmware version.
        @rtype: None
        @return: None
        '''
        self.capabilities.setcapab(KBCapabilities.SNIFF, True)
        self.capabilities.setcapab(KBCapabilities.SETCHAN, True)
        self.capabilities.setcapab(KBCapabilities.FREQ_2400, True)
        self.capabilities.setcapab(KBCapabilities.FREQ_900, True)
        return

    # KillerBee expects the driver to implement this function
    def get_dev_info(self):
	'''
        Returns device information in a list identifying the device.
        @rtype: List
        @return: List of 3 strings identifying device.
        '''
        return [self.dev, "Wislab Sniffer v{0}".format(self.__revision_num), getMacAddr(self.dev)]

    def __make_rest_call(self, path, fetch=True):
        '''
        Wrapper to the sniffer's RESTful services.
        Reports URL/HTTP errors as KBInterfaceErrors.
        @rtype: If fetch==True, returns a String of the page. Otherwise, it
            returns True if an HTTP 200 code was received.
        '''
        try:
            html = urllib2.urlopen("http://{0}/{1}".format(self.dev, path))
            if fetch:
                return html.read()
            else:
                return (html.getcode() == 200)
        except Exception as e:
            raise KBInterfaceError("Unable to preform a call to {0}/{1} (error: {2}).".format(self.dev, path, e))

    def __sniffer_status(self):
        '''
        Because the firmware accepts only toggle commands for sniffer on/off,
        we need to check what state it's in before taking action. It's also
        useful to make sure our command worked.
        @rtype: Boolean
        '''
        html = self.__make_rest_call('')
        # Yup, we're going to have to steal the status out of a JavaScript variable
        res = re.search(r'<!--#pindex-->([A-Z]+),', html)
        if res is None:
            raise KBInterfaceError("Unable to parse the sniffer's current status.")
        # RUNNING means it's sniffing, STOPPED means it's not.
        return (res.group(1) == "RUNNING")

    def __sync_status(self):
        '''
        This updates the standard self.__stream_open variable based on the 
        status as reported from asking the remote sniffer.
        '''
        self.__stream_open = self.__sniffer_status()

    def __sniffer_channel(self):
        '''
        Because the firmware accepts only toggle commands for sniffer on/off,
        we need to check what state it's in before taking action. It's also
        useful to make sure our command worked.
        @rtype: Boolean
        '''
        html = self.__make_rest_call('')
        # Yup, we're going to have to steal the channel number out of a JavaScript variable
        #  var values = removeSSItag('<!--#pindex-->RUNNING,00:1a:b6:00:0a:a4,10.10.10.2,0,High,0x0000,OFF,0,0').split(",");
        res = re.search(r'<!--#pindex-->[A-Z]+,[0-9a-f:]+,[0-9.]+,([0-9]+),', html)
        if res is None:
            raise KBInterfaceError("Unable to parse the sniffer's current channel.")
        return int(res.group(1))

    # KillerBee expects the driver to implement this function
    def sniffer_on(self, channel=None):
        '''
        Turns the sniffer on such that pnext() will start returning observed
        data.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SNIFF)

        # Because the Wislab just toggles, we have to only hit the page 
        # if we need to go from off to on state.
        self.__sync_status()
        if self.__stream_open == False:
            if channel != None:
                self.set_channel(channel)
            
            if not self.__make_rest_call('status.cgi?p=2', fetch=False):
                raise KBInterfaceError("Error instructing sniffer to start capture.")

            #This makes sure the change actually happened
            self.__sync_status()
            if not self.__stream_open:
                raise KBInterfaceError("Sniffer did not turn on capture.")
                
    # KillerBee expects the driver to implement this function
    def sniffer_off(self):
        '''
        Turns the sniffer off.
        @rtype: None
        '''
        # Because the Wislab just toggles, we have to only hit the page 
        # if we need to go from on to off state.
        self.__sync_status()
        if self.__stream_open == True:
            if not self.__make_rest_call('status.cgi?p=2', fetch=False):
                raise KBInterfaceError("Error instructing sniffer to stop capture.")
            
            #This makes sure the change actually happened
            self.__sync_status()
            if self.__stream_open:
                raise KBInterfaceError("Sniffer did not turn off capture.")
    
    @staticmethod
    def __get_default_modulation(channel):
        '''
        Return the Wislab-specific integer representing the modulation which
        should be choosen to be IEEE 802.15.4 complinating for a given channel 
        number.
        Captured values from sniffing Wislab web interface, unsure why these
        are done as such.
        Available modulations are listed at:
        http://www.sewio.net/open-sniffer/develop/http-rest-interface/
        @rtype: Integer, or None if unable to determine modulation
        '''
        if channel >= 11 or channel <= 26: return '0'   #O-QPSK 250 kb/s 2.4GHz
        elif channel >= 1 or channel <= 10: return 'c'  #O-QPSK 250 kb/s 915MHz
        elif channel >= 128 or channel <= 131: return '1c' #O-QPSK 250 kb/s 760MHz
        elif channel == 0: return '0'                   #O-QPSK 100 kb/s 868MHz
        else: return None                               #Error status

    # KillerBee expects the driver to implement this function
    def set_channel(self, channel):
        '''
        Sets the radio interface to the specifid channel (limited to 2.4 GHz channels 11-26)
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.SETCHAN)

        if self.capabilities.is_valid_channel(channel):
            # We only need to update our channel if it doesn't match the currently reported one.
            curChannel = self.__sniffer_channel()
            if channel != curChannel:
                self.modulation = self.__get_default_modulation(channel)
                print("Setting to channel {0}, modulation {1}.".format(channel, self.modulation))
                # Examples captured in fw v0.5 sniffing:
                #   channel 6, 250 compliant: http://10.10.10.2/settings.cgi?chn=6&modul=c&rxsens=0
                #   channel 12, 250 compliant: http://10.10.10.2/settings.cgi?chn=12&modul=0&rxsens=0
                #   chinese 0, 780 MHz, 250 compliant: http://10.10.10.2/settings.cgi?chn=128&modul=1c&rxsens=0
                #   chinese 3, 786 MHz, 250 compliant: http://10.10.10.2/settings.cgi?chn=131&modul=1c&rxsens=0
                self.__make_rest_call("settings.cgi?chn={0}&modul={1}&rxsens=0".format(channel, self.modulation), fetch=False)
                self._channel = self.__sniffer_channel()
            else:
                self._channel = curChannel
        else:
            raise Exception('Invalid channel number ({0}) was provided'.format(channel))

    # KillerBee expects the driver to implement this function
    def inject(self, packet, channel=None, count=1, delay=0):
        '''
        Not implemented.
        '''
        self.capabilities.require(KBCapabilities.INJECT)

    @staticmethod
    def __parse_zep_v2(data):
        '''
        Parse the packet from the ZigBee encapsulation protocol version 2/3 and 
        return the fields desired for usage by pnext().
        There is support here for some oddities specific to the Wislab 
        implementation of ZEP and the packet, such as CC24xx format FCS 
        headers being expected.
        
        The ZEP protocol parsing is mainly based on Wireshark source at:
        http://anonsvn.wireshark.org/wireshark/trunk/epan/dissectors/packet-zep.c
        * ZEP v2 Header will have the following format (if type=1/Data):
        *  |Preamble|Version| Type |Channel ID|Device ID|CRC/LQI Mode|LQI Val|NTP Timestamp|Sequence#|Reserved|Length|
        *  |2 bytes |1 byte |1 byte|  1 byte  | 2 bytes |   1 byte   |1 byte |   8 bytes   | 4 bytes |10 bytes|1 byte|
        * ZEP v2 Header will have the following format (if type=2/Ack):
        *  |Preamble|Version| Type |Sequence#|
        *  |2 bytes |1 byte |1 byte| 4 bytes |
        #define ZEP_PREAMBLE        "EX"
        #define ZEP_V2_HEADER_LEN   32
        #define ZEP_V2_ACK_LEN      8
        #define ZEP_V2_TYPE_DATA    1
        #define ZEP_V2_TYPE_ACK     2
        #define ZEP_LENGTH_MASK     0x7F
        '''
        # Unpack constant part of ZEPv2
        (preamble, version, zeptype) = unpack('<HBB', data[:4])
        if preamble != 22597 or version < 2: # 'EX'==22597, and v3 is compat with v2 (I think??)
            raise Exception("Can not parse provided data as ZEP due to incorrect preamble or unsupported version.")
        if zeptype == 1: #data
            (ch, devid, crcmode, lqival, ntpsec, ntpnsec, seqnum, length) = unpack(">BHBBIII10xB", data[4:32])
            #print "Data ZEP:", ch, devid, crcmode, lqival, ntpsec, ntpnsec, seqnum, length
            #We could convert the NTP timestamp received to system time, but the
            # Wislab firmware uses "relative timestamping" where it begins at 0 each time
            # the sniffer is started. Thus, it isn't that useful to us, so we just add the
            # time the packet is received at the host instead.
            #print "\tConverted time:", ntp_to_system_time(ntpsec, ntpnsec)
            recdtime = datetime.combine(date.today(), (datetime.now()).time()) #TODO address timezones by going to UTC everywhere
            #The LQI comes in ZEP, but the RSSI comes in the first byte of the FCS,
            # if the FCS was correct. If the byte is 0xb1, Wireshark appears to do 0xb1-256 = -79 dBm.
            # It appears that if CRC/LQI Mode field == 1, then checksum was bad, so the RSSI isn't
            # available, as the CRC is left in the packet. If it == 0, then the first byte of FCS is the RSSI.
            # From Wireshark:
            #define IEEE802154_CC24xx_CRC_OK            0x8000
            #define IEEE802154_CC24xx_RSSI              0x00FF
            frame = data[32:]
            # A length vs len(frame) check is not used here but is an 
            #  additional way to verify that all is good (length == len(frame)).
            if crcmode == 0:
                validcrc = ((ord(data[-1]) & 0x80) == 0x80)
                rssi = ord(data[-2])
                # We have to trust the sniffer that the FCS was OK, so we compute
                #  what a good FCS should be and patch it back into the packet.
                frame = frame[:-2] + makeFCS(frame[:-2])
            else:
                validcrc = False
                rssi = None
            return (frame, ch, validcrc, rssi, lqival, recdtime)
        elif zeptype == 2: #ack
            frame = data[8:]
            (seqnum) = unpack(">I", data[4:8])
            recdtime = datetime.combine(date.today(), (datetime.now()).time()) #TODO address timezones by going to UTC everywhere
            validcrc = (frame[-2:] == makeFCS(frame[:-2]))
            return (frame, None, validcrc, None, None, recdtime)
        return None

    # KillerBee expects the driver to implement this function
    def pnext(self, timeout=100):
        '''
        Returns a dictionary containing packet data, else None.
        @type timeout: Integer
        @param timeout: Timeout to wait for packet reception in usec
        @rtype: List
        @return: Returns None is timeout expires and no packet received.  When a packet is received, a dictionary is returned with the keys bytes (string of packet bytes), validcrc (boolean if a vaid CRC), rssi (unscaled RSSI), and location (may be set to None). For backwards compatibility, keys for 0,1,2 are provided such that it can be treated as if a list is returned, in the form [ String: packet contents | Bool: Valid CRC | Int: Unscaled RSSI ]
        '''
        if self.__stream_open == False:
            self.sniffer_on() #start sniffing

        # Use socket timeouts to implement the timeout
        self.handle.settimeout(timeout / 1000000.0) # it takes seconds

        frame = None
        donetime = datetime.now() + timedelta(microseconds=timeout)
        while True:
            try:
                data, addr = self.handle.recvfrom(1024)
            except error_timeout:
                return None
            # Ensure it's data coming from the right place, for now we just
            #  check the sending IP address. Ex: addr = ('10.10.10.2', 17754)
            if addr[0] != self.dev:
                continue
            # Dissect the UDP packet
            (frame, ch, validcrc, rssi, lqival, recdtime) = self.__parse_zep_v2(data)
            print "Valid CRC", validcrc, "LQI", lqival, "RSSI", rssi
            if frame == None or (ch is not None and ch != self._channel):
                #TODO this maybe should be an error condition, instead of ignored?
                print("ZEP parsing issue (bytes length={0}, channel={1}).".format(len(frame) if frame is not None else None, ch))
                continue
            break

        if frame is None:
            return None

        #Return in a nicer dictionary format, so we don't have to reference by number indicies.
        #Note that 0,1,2 indicies inserted twice for backwards compatibility.
        result = {0:frame, 1:validcrc, 2:rssi, 'bytes':frame, 'validcrc':validcrc, 'rssi':rssi, 'dbm':None, 'location':None, 'datetime':recdtime}
        if rssi is not None:
            result['dbm'] = rssi - 45 #TODO tune specifically to the platform (expect antenna varriances?)
        return result

    def jammer_on(self, channel=None):
        '''
        Not yet implemented.
        @type channel: Integer
        @param channel: Sets the channel, optional
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.PHYJAM)

    def jammer_off(self, channel=None):
        '''
        Not yet implemented.
        @return: None
        @rtype: None
        '''
        self.capabilities.require(KBCapabilities.PHYJAM)


import struct
import time

PCAPH_MAGIC_NUM = 0xa1b2c3d4
PCAPH_VER_MAJOR = 2
PCAPH_VER_MINOR = 4
PCAPH_THISZONE = 0
PCAPH_SIGFIGS = 0
PCAPH_SNAPLEN = 65535

class PcapReader:

    def __init__(self, savefile):
        '''
        Opens the specified file, validates a libpcap header is present.
        @type savefile: String
        @param savefile: Input libpcap filename to open
        @rtype: None
        '''
        PCAPH_LEN = 24
        self.__fh = open(savefile, mode='rb')
        self._pcaphsnaplen = 0
        header = self.__fh.read(PCAPH_LEN)

        # Read the first 4 bytes for the magic number, determine endianness
        magicnum = struct.unpack("I", header[0:4])[0]
        if magicnum != 0xd4c3b2a1:
            # Little endian
            self.__endflag = "<"
        elif magicnum == 0xa1b2c3d4:
            # Big endign
            self.__endflag = ">"
        else:
            raise Exception('Specified file is not a libpcap capture')

        pcaph = struct.unpack("%sIHHIIII"%self.__endflag, header)
        if pcaph[1] != PCAPH_VER_MAJOR and pcaph[2] != PCAPH_VER_MINOR \
                and pcaph[3] != PCAPH_THISZONE and pcaph[4] != PCAPH_SIGFIGS \
                and pcaph[5] != PCAPH_SNAPLEN:
            raise Exception('Unsupported pcap header format or version')

        self._pcaphsnaplen = pcaph[5]
        self._datalink = pcaph[6]

    def datalink(self):
        '''
        Returns the data link type for the packet capture.
        @rtype: Int
        '''
        return self._datalink

    def close(self):
        '''
        Closes the output packet capture; wrapper for pcap_close().
        @rtype: None
        '''
        self.pcap_close()

    def pcap_close(self):
        '''
        Closes the output packet capture.
        @rtype: None
        '''
        self.__fh.close()

    def pnext(self):
        '''
        Wrapper for pcap_next to mimic method for Daintree SNA.  See pcap_next()
        '''
        return self.pcap_next()
 
    def pcap_next(self):
        '''
        Retrieves the next packet from the capture file.  Returns a list of
        [Hdr, packet] where Hdr is a list of [timestamp, snaplen, plen] and
        packet is a string of the payload content.  Returns None at the end
        of the packet capture.
        @rtype: List
        '''
        # Read the next header block
        PCAPH_RECLEN = 16
        rechdrdata = self.__fh.read(PCAPH_RECLEN)

        try:
            rechdrtmp = struct.unpack("%sIIII"%self.__endflag, rechdrdata)
        except struct.error:
            return [None,None]

        rechdr = [
                float("%s.%s"%(rechdrtmp[0],rechdrtmp[1])), 
                rechdrtmp[2], 
                rechdrtmp[3]
                ]
        if rechdr[1] > rechdr[2] or rechdr[1] > self._pcaphsnaplen or rechdr[2] > self._pcaphsnaplen:
            raise Exception('Corrupted or invalid libpcap record header (included length exceeds actual length)')

        # Read the included packet length
        frame = self.__fh.read(rechdr[1])
        return [rechdr, frame]


class PcapDumper:

    def __init__(self, datalink, savefile, ppi = False):
        '''
        Creates a libpcap file using the specified datalink type.
        @type datalink: Integer
        @param datalink: Datalink type, one of DLT_* defined in pcap-bpf.h
        @type savefile: String
        @param savefile: Output libpcap filename to open
        @rtype: None
        '''
        if ppi: from killerbee.pcapdlt import DLT_PPI
        self.ppi = ppi
        self.__fh = open(savefile, mode='wb')
        self.datalink = datalink
        self.__fh.write(''.join([
            struct.pack("I", PCAPH_MAGIC_NUM), 
            struct.pack("H", PCAPH_VER_MAJOR),
            struct.pack("H", PCAPH_VER_MINOR),
            struct.pack("I", PCAPH_THISZONE),
            struct.pack("I", PCAPH_SIGFIGS),
            struct.pack("I", PCAPH_SNAPLEN),
            struct.pack("I", DLT_PPI if self.ppi else self.datalink)
            ]))

    def pcap_dump(self, packet, ts_sec=None, ts_usec=None, orig_len=None, 
                  freq_mhz = None, ant_dbm = None):
        '''
        Appends a new packet to the libpcap file.  Optionally specify ts_sec
        and tv_usec for timestamp information, otherwise the current time is
        used.  Specify orig_len if your snaplen is smaller than the entire
        packet contents.
        @type ts_sec: Integer
        @param ts_sec: Timestamp, number of seconds since Unix epoch.  Default
        is the current timestamp.
        @type ts_usec: Integer
        @param ts_usec: Timestamp microseconds.  Defaults to current timestamp.
        @type orig_len: Integer
        @param orig_len: Length of the original packet, used if the packet you
        are writing is smaller than the original packet.  Defaults to the
        specified packet's length.
        @type packet: String
        @param packet: Packet contents
        @rtype: None
        '''

        # Build CACE PPI headers if requested
        if self.ppi is True:
            pph_len = 8 + 24 #ppi_header + 802.11-common header and data
            rf_freq_mhz = 0x0000
            if freq_mhz is not None: rf_freq_mhz = freq_mhz
            rf_ant_dbm = 0
            if ant_dbm is not None: rf_ant_dbm = ant_dbm

            #CACE PPI Header
            caceppi_hdr = ''.join([
                struct.pack("<B", 0),
                struct.pack("<B", 0x01),
                struct.pack("<H", pph_len),
                struct.pack("<I", self.datalink)
                ])

            #CACE PPI Field 802.11-Common
            caceppi_f80211common = ''.join([
                struct.pack("<H", 2),        #2 = Field Type 802.11-Common
                struct.pack("<H", 20),       #20 = 802.11-Common length in bytes
                struct.pack("<Q", 0),        #FSF-Timer
                struct.pack("<H", 0),        #Flags
                struct.pack("<H", 0),        #Rate
                struct.pack("<H", rf_freq_mhz), #Channel-Freq
                struct.pack("<H", 0x0080),   #Channel-Flags = 2GHz
                struct.pack("<B", 0),        #FHSS-Hopset
                struct.pack("<B", 0),        #FHSS-Pattern
                struct.pack("<b", rf_ant_dbm),  #dBm-Ansignal
                struct.pack("<b", 0)         #dBm-Antnoise
                ])

        if ts_sec == None or ts_usec == None:
            # There must be a better way here that I don't know -JW
            s_sec, s_usec = str(time.time()).split(".")
            ts_sec = int(s_sec)
            ts_usec = int(s_usec)

        plen = len(packet)
        if orig_len == None:
            orig_len = plen

        #Encapsulated packet header and packet
        output_list = [ struct.pack("I", ts_sec),
                        struct.pack("I", ts_usec),
                        struct.pack("I", orig_len),
                        struct.pack("I", plen) ]

        if self.ppi is True:
            output_list[2] = struct.pack("I", orig_len + pph_len)
            output_list[3] = struct.pack("I", plen + pph_len)
            output_list.append(caceppi_hdr)
            output_list.append(caceppi_f80211common)

        output_list.append(packet)
        output = ''.join(output_list)

        #DEBUG Output:
        #print "Pcap:", '\\x'+'\\x'.join(["%02x" % ord(x) for x in output])
        #print "PPI:", '\\x'+'\\x'.join(["%02x" % ord(x) for x in (caceppi_hdr + caceppi_f80211common)])
        #print "802154:", packet.encode("hex")

        self.__fh.write(output)

        return


    def close(self):
        '''
        Closes the output packet capture; wrapper for pcap_close().
        @rtype: None
        '''
        self.pcap_close()

    def pcap_close(self):
        '''
        Closed the output packet capture.
        @rtype: None
        '''
        self.__fh.close()

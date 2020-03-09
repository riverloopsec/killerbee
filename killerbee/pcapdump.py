import struct
import time
from datetime import datetime

PCAPH_MAGIC_NUM = 0xa1b2c3d4
PCAPH_VER_MAJOR = 2
PCAPH_VER_MINOR = 4
PCAPH_THISZONE  = 0
PCAPH_SIGFIGS   = 0
PCAPH_SNAPLEN   = 65535

DOT11COMMON_TAG = 0o00002
GPS_TAG		= 30002

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
        @type savefile: String or file-like object
        @param savefile: Output libpcap filename to open, or file-like object
        @type ppi: Boolean
        @param ppi: Include CACE Per-Packet Information (defaults to False)
        @rtype: None
        '''
        if ppi: from killerbee.pcapdlt import DLT_PPI
        self.ppi = ppi

        if isinstance(savefile, str):
            self.__fh = open(savefile, mode='wb')
        elif hasattr(savefile, 'write'):
            self.__fh = savefile
        else:
            raise ValueError("Unsupported type for 'savefile' argument")

        self.datalink = datalink
        self.__fh.write(b"".join([
            struct.pack("I", PCAPH_MAGIC_NUM), 
            struct.pack("H", PCAPH_VER_MAJOR),
            struct.pack("H", PCAPH_VER_MINOR),
            struct.pack("I", PCAPH_THISZONE),
            struct.pack("I", PCAPH_SIGFIGS),
            struct.pack("I", PCAPH_SNAPLEN),
            struct.pack("I", DLT_PPI if self.ppi else self.datalink)
            ]))

    def __enter__(self):
        return self

    def __exit__(self, *exinfo):
        self.close()

    #TODO: fix freq_mhz for subGHz which end up as float
    def pcap_dump(self, packet, ts_sec=None, ts_usec=None, orig_len=None, 
                  freq_mhz = None, ant_dbm = None, location = None):
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
        @type location: Tuple
        @param location: 3-tuple of (longitude, latitude, altitude).
        @type packet: String
        @param packet: Packet contents
        @rtype: None
        '''

        # Build CACE PPI headers if requested
        if self.ppi is True:
            pph_len = 8 #ppi_header

            #CACE PPI Field 802.11-Common
            pph_len += 24 #802.11-common header and data
            rf_freq_mhz = 0x0000
            if freq_mhz is not None: rf_freq_mhz = int(freq_mhz) #TODO: fix for float
            rf_ant_dbm = 0
            if ant_dbm is not None: rf_ant_dbm = ant_dbm
            caceppi_f80211common = b"".join([
                struct.pack("<H", DOT11COMMON_TAG), #2 = Field Type 802.11-Common
                struct.pack("<H", 20),              #20 = 802.11-Common length in bytes
                struct.pack("<Q", 0),               #FSF-Timer
                struct.pack("<H", 0),               #Flags
                struct.pack("<H", 0),               #Rate
                struct.pack("<H", rf_freq_mhz),     #Channel-Freq
                struct.pack("<H", 0x0080),          #Channel-Flags = 2GHz
                struct.pack("<B", 0),               #FHSS-Hopset
                struct.pack("<B", 0),               #FHSS-Pattern
                struct.pack("<b", rf_ant_dbm),      #dBm-Ansignal
                struct.pack("<b", 0)                #dBm-Antnoise
                ])

            #PPI-GEOLOCATION if information available
            if location is not None:
                pph_len += 24 #geolocation header and data length
                (lon, lat, alt) = location
                # Sanity checking on values of location data:
                if lat > -180.00000005 and lat < 180.00000005:
                    lat_i = int(round((lat + 180.0) * 1e7))
                else:
                    raise Exception("Latitude value is out of expected range: %.8f" % lat)
                if lon > -180.00000005 and lon < 180.00000005:
                    lon_i = int(round((lon + 180.0) * 1e7))
                else:
                    raise Exception("Longitude value is out of expected range: %.8f" % lon)
                if alt > -180000.00005 and alt < 180000.00005:
                    alt_i = int(round((alt + 180000.0) * 1e4))
                else:
                    raise Exception("Altitude value is out of expected range: %.8f" % lon)
                # Build Geolocation PPI Header
                caceppi_fgeolocation = b"".join([
                    struct.pack("<H", GPS_TAG),  #2 = Field Type 802.11-Common
                    struct.pack("<H", 20),       #20 = 802.11-Common length in bytes
                    struct.pack("<B", 1),        #Geotag Version
                    struct.pack("<B", 2),        #Geotag Pad
                    struct.pack("<H", 24),       #Geotag Length
                    struct.pack("<I", 0x0E),     #GPS fields (only lat, long, and alt for now)
                    struct.pack("<I", lat_i),    #GPS Latitude
                    struct.pack("<I", lon_i),    #GPS Longitude
                    struct.pack("<I", alt_i),    #GPS Altitude
                    ])

            #CACE PPI Header
            caceppi_hdr = b"".join([
                struct.pack("<B", 0),		     #PPH version
                struct.pack("<B", 0x00),         #PPH flags
                struct.pack("<H", pph_len),	     #PPH len
                struct.pack("<I", self.datalink) #Field
                ])

        if ts_sec == None or ts_usec == None: 
            dt = datetime.now() 
            ts_sec = int(dt.strftime('%s')) 
            ts_usec = dt.microsecond

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
            if location is not None:
                output_list.append(caceppi_fgeolocation)
            output_list.append(caceppi_f80211common)

        output_list.append(packet)
        output = b''.join(output_list)

        self.__fh.write(output)
        # Specially for handling FIFO needs:
        try:
            self.__fh.flush()
        except IOError as e:
            raise e


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

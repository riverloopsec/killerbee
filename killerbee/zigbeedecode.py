import struct

#  ZigBee NWK FCF fields
ZBEE_NWK_FCF_FRAME_TYPE     = 0x0003 #: ZigBee NWK Frame Control Frame Type
ZBEE_NWK_FCF_VERSION        = 0x003C #: ZigBee NWK Frame Control Version
ZBEE_NWK_FCF_DISCOVER_ROUTE = 0x00C0 #: ZigBee NWK Frame Control Route Topology Discovery Flag
ZBEE_NWK_FCF_MULTICAST      = 0x0100 #: ZigBee NWK Frame Control Multicast Flag, ZigBee 2006 and Later
ZBEE_NWK_FCF_SECURITY       = 0x0200 #: ZigBee NWK Frame Control Security Bit
ZBEE_NWK_FCF_SOURCE_ROUTE   = 0x0400 #: ZigBee NWK Frame Control Source Route Bit, ZigBee 2006 and Later
ZBEE_NWK_FCF_EXT_DEST       = 0x0800 #: ZigBee NWK Frame Control Extended Destination Addressing, ZigBee 2006 and Later
ZBEE_NWK_FCF_EXT_SOURCE     = 0x1000 #: ZigBee NWK Frame Control Extended Source Addressing, ZigBee 2006 and Later

#  ZigBee NWK FCF Frame Types
ZBEE_NWK_FCF_DATA           = 0x0000 #: ZigBee NWK Frame Control Field Frame Type: Data
ZBEE_NWK_FCF_CMD            = 0x0001 #: ZigBee NWK Frame Control Field Frame Type: Command

# ZigBee APS FCF Fields
ZBEE_APS_FCF_FRAME_TYPE     = 0x03 #: ZigBee APS Frame Control Frame Type
ZBEE_APS_FCF_DELIVERY_MODE  = 0x0c #: ZigBee APS Frame Control Delivery Mode
ZBEE_APS_FCF_INDIRECT_MODE  = 0x10 #: ZigBee APS Frame Control Indirect Delivery Mode Flag, ZigBee 2004 and earlier.
ZBEE_APS_FCF_ACK_MODE       = 0x10 #: ZigBee APS Frame Control ACK Mode, ZigBee 2007 and later.
ZBEE_APS_FCF_SECURITY       = 0x20 #: ZigBee APS Frame Control Security Bit
ZBEE_APS_FCF_ACK_REQ        = 0x40 #: ZigBee APS Frame Control ACK Required Bit
ZBEE_APS_FCF_EXT_HEADER     = 0x80 #: ZigBee APS Frame Control Extended Header Bit

ZBEE_APS_FCF_DATA           = 0x00 #: ZigBee APS Frame Control Field Frame Type: Data
ZBEE_APS_FCF_CMD            = 0x01 #: ZigBee APS Frame Control Field Frame Type: Command
ZBEE_APS_FCF_ACK            = 0x02 #: ZigBee APS Frame Control Field Frame Type: ACK

ZBEE_APS_FCF_UNICAST        = 0x00 #: ZigBee APS Frame Control Field Delivery Mode: Unicast Delivery
ZBEE_APS_FCF_INDIRECT       = 0x01 #: ZigBee APS Frame Control Field Delivery Mode: Indirect Delivery
ZBEE_APS_FCF_BCAST          = 0x02 #: ZigBee APS Frame Control Field Delivery Mode: Broadcast Delivery
ZBEE_APS_FCF_GROUP          = 0x03 #: ZigBee APS Frame Control Field Delivery Mode: Group Delivery, ZigBee 2006 and later. 

# ZigBee Security Levels
ZBEE_SEC_ENC_MIC_32         = 0x5  #: ZigBee Seecurity Level id 5; Encrypted, 4 byte MIC

class ZigBeeNWKPacketParser:
    def __init__(self):
        '''
        Instantiates the ZigBeeNWKPacketParser class.
        '''

        return

    def pktchop(self, packet):
        '''
        Chops up the specified packet contents into a list of fields.  Does
        not attempt to re-order the field values for parsing.  ''.join(X) will
        reassemble original packet string.  Fields which may or may not be
        present (such as the destination address) are empty if they are not
        present, keeping the list elements consistent, as follows:
        Frame Control | DA | SA | Radius | Seq # | Dst IEEE Address | Src IEEE Address | MCast Ctrl | Src Route Subframe | Payload

        An exception is raised if the packet contents are too short to
        decode.

        @type packet: String
        @param packet: Packet contents.
        @rtype: list
        @return: Chopped contents of the ZigBee NWK packet into list elements.
        '''
        if len(packet) < 8:
            raise Exception("Packet too small, %d bytes." % len(packet))
        
        # Frame control field
        fc = struct.unpack("<H",packet[0:2])[0]

        # FC | DA | SA | Radius | Seq #
        pktchop = [packet[0:2], packet[2:4], packet[4:6], packet[6], packet[7]]
        offset = 8

        # Check if the DA bit is set in the frame control field
        if (fc & ZBEE_NWK_FCF_EXT_DEST) != 0:
            pktchop.append(packet[offset:offset+8])
            offset+=8
        else:
            pktchop.append("")

        # Check if the SA bit is set in the frame control field
        if (fc & ZBEE_NWK_FCF_EXT_SOURCE) != 0:
            pktchop.append(packet[offset:offset+8])
            offset+=8
        else:
            pktchop.append("")

        # Check if the Multicast Control bit is set in the frame control field
        if (fc & ZBEE_NWK_FCF_MULTICAST) != 0:
            pktchop.append(packet[offset])
            offset+=1
        else:
            pktchop.append("")

        # Check if the Source Route bit is set in the frame control field
        if (fc & ZBEE_NWK_FCF_SOURCE_ROUTE) != 0:
            # Source Route Subfield has a format Count | Index | List
            # where Count and Index are 1-byte each, and list is a list of
            # addresses for source routing count*2 long.
            relaycount = packet[offset]
            relayindex = packet[offset+1]
            relaylist = packet[offset+2:(relaycount*2)]
            sourceroutesubframe = packet[relaycount + relayindex + relaylist]
            pktchop.append(sourceroutesubframe)
            offset+=len(sourceroutesubframe)
        else:
            pktchop.append("")

        # Append remaining payload
        pktchop.append(packet[offset:])

        return pktchop
        
    def hdrlen(self, packet):
        '''
        Returns the length of the ZigBee NWK header.
        @type packet: String
        @param packet: Packet contents to evaluate for header length.
        @rtype: Int
        @return: Length of the ZigBEE NWK header.
        '''
        # Frame control field
        fc = struct.unpack("<H",packet[0:2])[0]
        plen = 8

        # Check if the DA bit is set in the frame control field
        if (fc & ZBEE_NWK_FCF_EXT_DEST) != 0:
            plen+=8

        # Check if the SA bit is set in the frame control field
        if (fc & ZBEE_NWK_FCF_EXT_SOURCE) != 0:
            plen+=8

        # Check if the Source Route bit is set in the frame control field
        if (fc & ZBEE_NWK_FCF_SOURCE_ROUTE) != 0:
            # relay list based on relay count * 2
            plen+= (ord(packet[plen])*2)
            # relay count and relay index fields
            plen+=2 
        
        return plen

    def payloadlen(self, packet):
        '''
        Returns the length of the NWK payload.
        @type packet: String
        @param packet: Packet contents to evaluate for header length.
        @rtype: Int
        @return: Length of the NWK payload.
        '''
        return len(packet) - self.hdrlen(packet)



class ZigBeeAPSPacketParser:
    def __init__(self):
        '''
        Instantiates the ZigBeeAPSPacketParser class.
        '''

        return

    def pktchop(self, packet):
        '''
        Chops up the specified packet contents into a list of fields.  Does
        not attempt to re-order the field values for parsing.  ''.join(X) will
        reassemble original packet string.  Fields which may or may not be
        present (such as the destination endpoint) are empty if they are not
        present, keeping the list elements consistent, as follows:
        Frame Control | Dst Endpoint | Group Address | Cluster Identifier | Profile Identifier | Source Endpoint | APS Counter | Fragmentation | Block Number | Payload

        An exception is raised if the packet contents are too short to
        decode.

        @type packet: String
        @param packet: Packet contents.
        @rtype: list
        @return: Chopped contents of the ZigBee APS packet into list elements.
        '''
        if len(packet) < 3:
            raise Exception("Packet too small, %d bytes." % len(packet))
        
        # Frame control field
        fc = packet[0]
        pktchop = [fc,]
        offset = 1

        # Identify the APS frame type
        apsftype = ord(fc) & ZBEE_APS_FCF_FRAME_TYPE
        apsdeliverymode = (ord(fc) & ZBEE_APS_FCF_DELIVERY_MODE >> 2)

        if apsftype == ZBEE_APS_FCF_DATA:
            # APS Data frames are FC | Dst Endpoint | Group Address | Cluster ID | Src Endpoint | APS Counter

            # Get Delivery Mode value
            if apsdeliverymode == 0:        # Normal Unicast Delivery
                pktchop.append(packet[offset])              # Dst Endpoint
                pktchop.append("")                          # Group Address (not present in this mode)
                pktchop.append(packet[offset+1:offset+3])   # Cluster ID
                pktchop.append(packet[offset+3:offset+5])   # Profile ID
                pktchop.append(packet[offset+5])            # Source Endpoint
                offset += 6
            elif apsdeliverymode == 1:   # Indirect Delivery
                pktchop.append("")                          # Dst Endpoint (not present)
                pktchop.append("")                          # Group Address (not present)
                pktchop.append(packet[offset:offset+2])     # Cluster ID
                pktchop.append(packet[offset+2:offset+4])   # Profile ID
                pktchop.append(packet[offset+4])            # Source Endpoint
                offset += 5
            elif apsdeliverymode == 2:   # Broadcast Delivery
                pktchop.append(packet[offset])              # Dst Endpoint
                pktchop.append("")                          # Group Address (not present)
                pktchop.append(packet[offset+1:offset+3])   # Cluster ID
                pktchop.append(packet[offset+3:offset+5])   # Profile ID
                pktchop.append(packet[offset+5])            # Source Endpoint
                offset += 6
            else:           # Group Delivery
                pktchop.append("")                          # Dst Endpoint 
                pktchop.append(packet[offset:offset+2])     # Group Address (not present)
                pktchop.append(packet[offset+2:offset+4])   # Cluster ID
                pktchop.append(packet[offset+4:offset+6])   # Profile ID
                pktchop.append(packet[offset+6])            # Source Endpoint
                offset += 7

        elif apsftype == ZBEE_APS_FCF_CMD:
            # APS Command frames are FC | Group Address | APS Counter
            pktchop.append("")                          # Dst Endpoint (not present)

            # Get Delivery Mode value
            if apsdeliverymode == 2:      # Group Delivery
                pktchop.append(packet[offset:offset+2])     # Group Address
                offset += 2
            else:
                pktchop.append("")                          # Group Address (not present)

            pktchop.append("")                          # Cluster ID (not present)
            pktchop.append("")                          # Source Endpoint (not present)

        elif apsftype == ZBEE_APS_FCF_ACK:
            if apsdeliverymode == 0:        # Normal Unicast Delivery
                pktchop.append(packet[offset])              # Dst Endpoint
                pktchop.append("")                          # Group Address (not present in this mode)
                pktchop.append(packet[offset+1:offset+3])   # Cluster ID
                pktchop.append(packet[offset+3:offset+5])   # Profile ID
                pktchop.append(packet[offset+5])            # Source Endpoint
                offset += 6
            elif apsdeliverymode == 1:   # Indirect Delivery
                pktchop.append(packet[offset])              # Dst Endpoint
                pktchop.append("")                          # Group Address (not present)
                pktchop.append(packet[offset+1:offset+3])     # Cluster ID
                pktchop.append(packet[offset+3:offset+5])   # Profile ID
                pktchop.append(packet[offset+5])            # Source Endpoint
                offset += 6
            elif apsdeliverymode == 2:   # Broadcast Delivery
                pktchop.append(packet[offset])              # Dst Endpoint
                pktchop.append("")                          # Group Address (not present)
                pktchop.append(packet[offset+1:offset+3])   # Cluster ID
                pktchop.append(packet[offset+3:offset+5])   # Profile ID
                pktchop.append(packet[offset+5])            # Source Endpoint
                offset += 6
            else:           # Group Delivery
                pktchop.append(packet[offset])              # Dst Endpoint
                pktchop.append(packet[offset:offset+3])     # Group Address (not present)
                pktchop.append(packet[offset+3:offset+5])   # Cluster ID
                pktchop.append(packet[offset+5:offset+7])   # Profile ID
                pktchop.append(packet[offset+7])            # Source Endpoint
                offset += 8

        # APS Counter
        pktchop.append(packet[offset])
        offset+= 1

        # Extended Header (fragmentation)
        if ord(fc) & ZBEE_APS_FCF_EXT_HEADER:
            # if fragmentation is set get another byte as block number
            if packet[offset] != 0x00:
                pktchop.append(packet[offset])
                offset+= 1
            pktchop.append(packet[offset])
            offset+= 1
        else:
            pktchop.append("") # fragmentation
            pktchop.append("") # block number

        # Payload
        pktchop.append(packet[offset:])
        return pktchop
        
    def hdrlen(self, packet):
        '''
        Returns the length of the ZigBee NWK header.
        @type packet: String
        @param packet: Packet contents to evaluate for header length.
        @rtype: Int
        @return: Length of the ZigBEE NWK header.
        '''
        # Frame control field
        fc = packet[0]
        pktchop = [fc,]
        plen = 1

        # Identify the APS frame type
        apsftype = ord(fc) & ZBEE_APS_FCF_FRAME_TYPE
        apsdeliverymode = (ord(fc) & ZBEE_APS_FCF_DELIVERY_MODE >> 2)

        if apsftype == ZBEE_APS_FCF_DATA:
            if apsdeliverymode == 0 or apsdeliverymode == 2:
                plen += 6
            elif apsdeliverymode == 1:
                plen += 5
            else:
                plen += 7

        elif apsftype == ZBEE_APS_FCF_CMD:
            if apsdeliverymode == 2:
                plen += 2

        elif apsftype == ZBEE_APS_FCF_ACK:
            if apsdeliverymode == 3:
                plen += 8
            else:
                plen += 6
        
        plen += 1 # APS Counter

        # fragmentation + packet number if true
        if ord(fc) & ZBEE_APS_FCF_EXT_HEADER:
            if packet[plen] != 0x00:
                plen += 1
            plen += 1

        return plen

    def payloadlen(self, packet):
        '''
        Returns the length of the APS payload.
        @type packet: String
        @param packet: Packet contents to evaluate for header length.
        @rtype: Int
        @return: Length of the APS payload.
        '''
        return len(packet) - self.hdrlen(packet)

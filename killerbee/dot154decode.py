import struct  # type: ignore

from Crypto.Cipher import AES  # type: ignore

## Constants for packet decoding fields
# Frame Control Field
DOT154_FCF_TYPE_MASK            = 0x0007  #: Frame type mask
DOT154_FCF_SEC_EN               = 0x0008  #: Set for encrypted payload
DOT154_FCF_FRAME_PND            = 0x0010  #: Frame pending
DOT154_FCF_ACK_REQ              = 0x0020  #: ACK request
DOT154_FCF_INTRA_PAN            = 0x0040  #: Intra-PAN activity
DOT154_FCF_DADDR_MASK           = 0x0C00  #: Destination addressing mode mask
DOT154_FCF_VERSION_MASK         = 0x3000  #: Frame version
DOT154_FCF_SADDR_MASK           = 0xC000  #: Source addressing mask mode

# Frame Control Field Bit Shifts
DOT154_FCF_TYPE_MASK_SHIFT      = 0  #: Frame type mask mode shift
DOT154_FCF_DADDR_MASK_SHIFT     = 10  #: Destination addressing mode mask
DOT154_FCF_VERSION_MASK_SHIFT   = 12  #: Frame versions mask mode shift
DOT154_FCF_SADDR_MASK_SHIFT     = 14  #: Source addressing mask mode shift

# Address Mode Definitions
DOT154_FCF_ADDR_NONE            = 0x0000  #: Not sure when this is used
DOT154_FCF_ADDR_SHORT           = 0x0002  #: 4-byte addressing
DOT154_FCF_ADDR_EXT             = 0x0003  #: 8-byte addressing

DOT154_FCF_TYPE_BEACON          = 0     #: Beacon frame
DOT154_FCF_TYPE_DATA            = 1     #: Data frame
DOT154_FCF_TYPE_ACK             = 2     #: Acknowledgement frame
DOT154_FCF_TYPE_MACCMD          = 3     #: MAC Command frame

DOT154_CRYPT_NONE               = 0x00    #: No encryption, no MIC
DOT154_CRYPT_MIC32              = 0x01    #: No encryption, 32-bit MIC
DOT154_CRYPT_MIC64              = 0x02    #: No encryption, 64-bit MIC
DOT154_CRYPT_MIC128             = 0x03    #: No encryption, 128-bit MIC
DOT154_CRYPT_ENC                = 0x04    #: Encryption, no MIC
DOT154_CRYPT_ENC_MIC32          = 0x05    #: Encryption, 32-bit MIC
DOT154_CRYPT_ENC_MIC64          = 0x06    #: Encryption, 64-bit MIC
DOT154_CRYPT_ENC_MIC128         = 0x07    #: Encryption, 128-bit MIC

class Dot154PacketParser:
    def __init__(self):
        """
        Instantiates the Dot154PacketParser class.
        """

        # State values for AES-CTR mode
        self.__crypt_blockcntr = 1
        self.__crypt_A_i = []
        return

    def __crypt_counter(self):
        """
        Used for AES-CTR mode after populating self.__crypt_A_i
        Don't call this directly.  Just don't.
        """
        retindex = self.__crypt_blockcntr
        self.__crypt_blockcntr += 1
        return self.__crypt_A_i[retindex]

    def decrypt(self, packet, key):
        """
        Decrypts the specified packet. Returns empty byte if the packet is
        not encrypted, or if decryption MIC validation fails.

        @type packet: Bytes
        @param packet: Packet contents.
        @type key: Bytes
        @param key: Key contents.
        @rtype: Bytes
        @return: Decrypted packet contents, empty byte if not encrypted or if
        decryped MIC fails validation.
        """

        # Retrieve the data payload from the packet contents
        encpayload = packet[-self.payloadlen(packet) :]

        if encpayload[0] != DOT154_CRYPT_ENC_MIC64:
            raise UnsupportedPacket(
                "Unsupported security level in packet: 0x%02x." % encpayload[0]
            )

        if len(key) != 16:
            raise BadKeyLength("Invalid key length (%d)." % len(key))

        # Encrypted content is:
        # Sec Level | 4-byte counter | Flags | Ciphertext | Encrypted 8-byte MIC

        if self.payloadlen(packet) < 15:
            raise BadPayloadLength(
                "Payload length too short (%d)." % self.payloadlen(packet)
            )

        nonce = self.nonce(packet)

        # c = ciphertext payload including trailing 8-byte encrypted MIC
        c = encpayload[-9:]

        # 1. Parse C||U where U is the right-most bytes for MIC and C is the
        #    remaining bytes (representing encrypted packet payload content)
        C = c[0:-8]
        U = c[-8:]

        # 2. Form cipherText by padding C to a block size
        cipherText = C + (b"\x00" * (16 - len(C) % 16))

        # 3. Form 1-byte flags field = 01
        #    XXX will vary when L changes
        flags = b"\x01"

        # 4. Define 16-octet A_i consisting of:
        #        Flags || Nonce || 2-byte counter i for i=0,1,2, ...
        #    A[0] is for authenticity check, A[1] is for the first block of data,
        #    A[2] is for the 2nd block of data, if C > 16
        self.__crypt_A_i = []
        for i in range(0, (1 + 1 + int(len(C) / 16))):
            self.__crypt_A_i.append(flags + nonce + struct.pack(">H", i))

        # 5. Decrypt cipherText producing plainText (observed)
        self.__crypt_blockcntr = 1  # Start at A[1] to decrypt
        crypt = AES.new(key, AES.MODE_CTR, counter=self.__crypt_counter)

        plainText = crypt.decrypt(cipherText)[0 : len(C)]

        # 6. Compute S_0 as E(Key, A[0])
        crypt = AES.new(key, AES.MODE_CBC, b"\x00" * 16)
        S_0 = crypt.encrypt(self.__crypt_A_i[0])

        # 7. Compute MIC (T) observed as S_0 XOR U
        T_obs = []
        for i in range(0, len(S_0[0:8])):
            T_obs.append((S_0[i] ^ U[i]))

        # Convert T_obs back into a string (please, I need Python help)
        T_obs = b"".join(struct.pack("B", i) for i in T_obs)

        # 8. Compute a over packet contents before ciphertext payload
        #    This is the 802.15.4 header,plus the security level, frame
        #    counter and flags byte (01)
        hdrlen = self.hdrlen(packet)
        a = packet[0:hdrlen] + packet[hdrlen : hdrlen + 6]

        # 9. Concatenate L(a) of 2-byte length a with a
        addAuthData = struct.pack(">H", len(a)) + a

        # 10. Pad addAuthData to an even block size
        addAuthData += b"\x00" * (16 - len(addAuthData) % 16)

        # 11. Form AuthData by concatenating addAuthData and PlaintextData
        #     Pad plainText to an even block size
        plainTextPadded = plainText + (b"\x00" * (16 - len(plainText) % 16))

        authData = addAuthData + plainTextPadded

        # 12. Perform authData transformation into B[0], B[1], ..., B[i]
        B = b"\x59" + nonce + b"\x00\x01" + authData

        # 13. Calculate the MIC (T) calculated with CBC-MAC
        iv = b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

        for i in range(0, int(len(B) / 16)):
            crypt = AES.new(key, AES.MODE_CBC, iv)
            Bn = B[i * 16 : (i * 16) + 16]
            iv = crypt.encrypt(Bn)

        T_calc = iv[0:8]

        # 14. Compare
        if T_obs == T_calc:
            return plainText
        else:
            return ""

    def pktchop(self, packet):
        """
        Chops up the specified packet contents into a list of fields.  Does
        not attempt to re-order the field values for parsing.  ''.join(X) will
        reassemble original packet string.  Fields which may or may not be
        present (such as the Source PAN field) are empty if they are not
        present, keeping the list elements consistent, as follows:
        FCF | Seq# | DPAN | DA | SPAN | SA | [Beacon Data] | PHY Payload

        If the packet is a beacon frame, the Beacon Data field will be populated
        as a list element in the format:

        Superframe Spec | GTS Fields | Pending Addr Counts | Proto ID | Stack Profile/Profile Version | Device Capabilities | Ext PAN ID | TX Offset | Update ID

        An exception is raised if the packet contents are too short to
        decode.

        @type packet: Bytes
        @param packet: Packet contents.
        @rtype: list
        @return: Chopped contents of the 802.15.4 packet into list elements.
        """
        pktchop = ["", "", "", "", "", "", [], ""]

        pktchop[0] = packet[0:2]

        # Sequence number
        pktchop[1] = struct.pack(">B", packet[2])

        # Byte swap
        fcf = struct.unpack("<H", pktchop[0])[0]

        # Check if we are dealing with a beacon frame
        if (fcf & DOT154_FCF_TYPE_MASK) == DOT154_FCF_TYPE_BEACON:

            beacondata = ["", "", "", "", "", "", "", "", "", ""]

            try:

                # 802.15.4 fields, SPAN and SA
                pktchop[4] = packet[3:5]
                pktchop[5] = packet[5:7]
                offset = 7

                # Superframe specification
                beacondata[0] = packet[offset : offset + 2]
                offset += 2

                # GTS data
                beacondata[1] = packet[offset]
                offset += 1

                # Pending address count
                beacondata[2] = packet[offset]
                offset += 1

                # Protocol ID
                beacondata[3] = packet[offset]
                offset += 1

                # Stack Profile version
                beacondata[4] = packet[offset]
                offset += 1

                # Capability information
                beacondata[5] = packet[offset]
                offset += 1

                # Extended PAN ID
                beacondata[6] = packet[offset : offset + 8]
                offset += 8

                # TX Offset
                beacondata[7] = packet[offset : offset + 3]
                offset += 3

                # Update ID
                beacondata[8] = packet[offset]
                offset += 1

            except:
                pass

            pktchop[6] = beacondata

        else:
            # Not a beacon frame

            # DPAN
            pktchop[2] = packet[3:5]
            offset = 5

            # Examine the destination addressing mode
            daddr_mask = (fcf & DOT154_FCF_DADDR_MASK) >> 10

            if daddr_mask == DOT154_FCF_ADDR_EXT:
                pktchop[3] = packet[offset : offset + 8]
                offset += 8
            elif daddr_mask == DOT154_FCF_ADDR_SHORT:
                pktchop[3] = packet[offset : offset + 2]
                offset += 2

            # Examine the Intra-PAN flag
            if (fcf & DOT154_FCF_INTRA_PAN) == 0:
                pktchop[4] = packet[offset : offset + 2]
                offset += 2

            # Examine the source addressing mode
            saddr_mask = (fcf & DOT154_FCF_SADDR_MASK) >> 14

            if daddr_mask == DOT154_FCF_ADDR_EXT:
                pktchop[5] = packet[offset : offset + 8]
                offset += 8
            elif daddr_mask == DOT154_FCF_ADDR_SHORT:
                pktchop[5] = packet[offset : offset + 2]
                offset += 2

        # Append remaining payload
        if offset < len(packet):
            pktchop[7] = packet[offset:]

        return pktchop

    def hdrlen(self, packet):
        """
        Returns the length of the 802.15.4 header.
        @type packet: Bytes
        @param packet: Packet contents to evaluate for header length.
        @rtype: Int
        @return: Length of the 802.15.4 header.
        """

        # Minimum size is 11 (2 bytes FCF + 1 byte SEQ + 2 bytes DPAN +
        #  2 bytes DstAddr + 2 bytes SPAN + 2 bytes SrcAddr)
        # XXX Need to validate this logic based on specification
        if len(packet) < 9:
            raise Exception("Packet too small, %d bytes." % len(packet))

        # Start with minimum size, increase as needed based on FCF flags
        plen = 9

        # Byte swap
        fcf = struct.unpack("<H", packet[0:2])[0]

        # Examine the destination addressing mode
        if (fcf & DOT154_FCF_DADDR_MASK) >> 10 == DOT154_FCF_ADDR_EXT:
            plen += 6  # 8-byte addressing is in use, increasing addr 6 bytes

        # Examine the source addressing mode
        if (fcf & DOT154_FCF_SADDR_MASK) >> 14 == DOT154_FCF_ADDR_EXT:
            plen += 6  # 8-byte addressing is in use, increasing addr 6 bytes

        # Examine the Intra-PAN flag
        if (fcf & DOT154_FCF_INTRA_PAN) == 0:
            plen += 2  # Intra-PAN is false, source PAN 2-bytes is present

        return plen

    def payloadlen(self, packet):
        """
        Returns the length of the 802.15.4 payload.
        @type packet: Bytes
        @param packet: Packet contents to evaluate for header length.
        @rtype: Int
        @return: Length of the 802.15.4 payload.
        """
        return len(packet) - self.hdrlen(packet)

    def nonce(self, packet):
        """
        Returns the nonce of the 802.15.4 packet.  Returns empty string for
        unencrypted frames.
        @type packet: Bytes
        @param packet: Packet contents to evaluate for nonce.
        @rtype: Bytes
        @return: Nonce, empty when the frame is not encrypted.
        """

        # Byte swap
        fcf = struct.unpack("<H", packet[0:2])[0]

        if (fcf & DOT154_FCF_SEC_EN) == 0:
            # Packet is not encrypted
            return ""

        # Nonce formation is Src Addr || Frame Counter || Security Level
        pchop = self.pktchop(packet)

        # SA is the 5th list element, reverse it
        noncep1 = pchop[5][::-1]

        # Retrieve the data payload from the packet contents
        encpayload = packet[-self.payloadlen(packet) :]

        # First byte of encrypted payload is the security level
        noncep3 = struct.pack("<B", encpayload[0])

        # The next 4 bytes of the encrypted payload is the frame counter, rev
        noncep2 = encpayload[1:5][::-1]

        return noncep1 + noncep2 + noncep3

class UnsupportedPacket(Exception):
  pass

class BadKeyLength(Exception):
  pass

class BadPayloadLength(Exception):
  pass

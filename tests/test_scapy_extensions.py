import unittest
import struct
import argparse
import os
from scapy.all import *
from killerbee.scapy_extensions import * 

class TestKbScapyExt(unittest.TestCase):
    def test_kbdev(self):
        kbdev()
        self.assertTrue(True)

    def test_kbsendp(self):
        devstring = os.environ['APIMOTE_DEVSTRING']
        packet = Dot15d4(struct.pack("<80s", (b'\x01'*80)))
        kbsendp(packet)
        self.assertTrue(True)

    def _test_kbsrp(self):
        devstring = os.environ['APIMOTE_DEVSTRING']
        packet = Dot15d4(struct.pack("<80s", (b'\x01'*80)))
        kbsrp(packet)
        self.assertTrue(True)
        
    def _test_kbsrp1(self):
        devstring = os.environ['APIMOTE_DEVSTRING']
        packet = Dot15d4(struct.pack("<80s", (b'\x01'*80)))
        kbsrp1(packet)
        self.assertTrue(True)
        
    def _test_kbsniff(self):
        devstring = os.environ['APIMOTE_DEVSTRING']
        kbsniff(channel=11)
        self.assertTrue(True)

    def _test_kbrdpcap(self):
        path_to_file = "./tests/fixtures/test_pcap.pcap"

        kbrdpcap(path_to_file)

        self.assertTrue(True)

    def _test_kbwrpcap(self):
        path_to_file = "./tests/fixtures/test_pcap.pcap"
        packets = kbrdpcap(path_to_file)
        kbwrpcap(path_to_file, packets)

        self.assertTrue(True)

    #daintree deprecated
    def _test_kbrddain(self):
        self.assertFalse(False)
    def _test_kbwrdain(self):
        self.assertFalse(False)

    # TODO get binary keyfile and encrypted/decrypted packets
    def _test_kbkeysearch(self):
        self.assertTrue(True)

    def _test_kbgetnetworkkey(self):
        path_to_file = "./tests/fixtures/test_pcap.pcap"
        network_key =  b"\xe4\x2f\xc8\x5f\x9b\xaf\x5f\x4f\xb5\x8e\x6a\x1d\xbd\x2a\x13\x45"
        packets = kbrdpcap(path_to_file)
        recovered_key = kbgetnetworkkey(packets)
        print(recovered_key)

        self.assertEqual(network_key, recovered_key)

    #TODO figure out how to test
    def _test_kbtshark(self):
        self.assertTrue(True)

    def test_kbrandmac(self):
        self.assertEqual(8, len(kbrandmac()))
        self.assertEqual(12, len(kbrandmac(12)))
        self.assertEqual(6, len(kbrandmac(6)))
        self.assertEqual(3, len(kbrandmac(3)))

    def test_kbdecrypt(self):
        network_key =  b"\xe4\x2f\xc8\x5f\x9b\xaf\x5f\x4f\xb5\x8e\x6a\x1d\xbd\x2a\x13\x45"
        encrypted_packet = b'a\x88\xe0|F\x00\x00s\xc5H"\x00\x00s\xc5\x1e\x00(\xee\xee\x00\x00\x8a\xdc\x1a\xfe\xff\x81\x8eX\x00\xcf\x8c\xce6\x16\xe7\x8cA)\x87\x12\x84#2\xd8_y\xe7~\x8d\xfb\xf8'
        plaintext_data = b'@\x01\x01\x05\x04\x01\x01Y\x01\x00\x00\x00\x041234\x00'
        scapy_packet = Dot15d4(encrypted_packet)

        decrypted_data = kbdecrypt(scapy_packet, network_key)

        self.assertEqual(plaintext_data, decrypted_data.build())

    def test_kbencrypt(self):
        network_key =  b"\xe4\x2f\xc8\x5f\x9b\xaf\x5f\x4f\xb5\x8e\x6a\x1d\xbd\x2a\x13\x45"
        encrypted_packet = b'a\x88\xe0|F\x00\x00s\xc5H"\x00\x00s\xc5\x1e\x00(\xee\xee\x00\x00\x8a\xdc\x1a\xfe\xff\x81\x8eX\x00\xcf\x8c\xce6\x16\xe7\x8cA)\x87\x12\x84#2\xd8_y\xe7~\x8d\xfb\xf8'
        data = b'@\x01\x01\x05\x04\x01\x01Y\x01\x00\x00\x00\x041234\x00'
        packet = b'a\x88\xe0|F\x00\x00s\xc5H"\x00\x00s\xc5\x1e\x00(\xee\xee\x00\x00\x8a\xdc\x1a\xfe\xff\x81\x8eX\x00@\x01\x01\x05\x04\x01\x01Y\x01\x00\x00\x00\x041234\x00'
        encrypted_data = kbencrypt(Dot15d4(packet), data, network_key)

        self.assertEqual(encrypted_data.build(), encrypted_packet)

    def test_kbgetmiclen(self):
        self.assertEqual(0, kbgetmiclen(DOT154_CRYPT_NONE))
        self.assertEqual(4, kbgetmiclen(DOT154_CRYPT_MIC32))
        self.assertEqual(8, kbgetmiclen(DOT154_CRYPT_MIC64))
        self.assertEqual(16, kbgetmiclen(DOT154_CRYPT_MIC128))
        self.assertEqual(0, kbgetmiclen(DOT154_CRYPT_ENC))
        self.assertEqual(4, kbgetmiclen(DOT154_CRYPT_ENC_MIC32))
        self.assertEqual(8, kbgetmiclen(DOT154_CRYPT_ENC_MIC64))
        self.assertEqual(16, kbgetmiclen(DOT154_CRYPT_ENC_MIC128))
       
    def _test_kbgetpanid(self):
        self.assertTrue(True)
    
if __name__ == "__main__":
    unittest.main()


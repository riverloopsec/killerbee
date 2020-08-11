import unittest
import struct
import argparse
import os

from killerbee.kbutils import * 

class TestKbutils(unittest.TestCase):
    def test_kbcapabilities_init(self):
        kbc=KBCapabilities()
    
        self.assertFalse(kbc._capabilities[KBCapabilities.NONE])
        self.assertFalse(kbc._capabilities[KBCapabilities.SNIFF])
        self.assertFalse(kbc._capabilities[KBCapabilities.SETCHAN])
        self.assertFalse(kbc._capabilities[KBCapabilities.INJECT])
        self.assertFalse(kbc._capabilities[KBCapabilities.PHYJAM])
        self.assertFalse(kbc._capabilities[KBCapabilities.SELFACK])
        self.assertFalse(kbc._capabilities[KBCapabilities.PHYJAM_REFLEX])
        self.assertFalse(kbc._capabilities[KBCapabilities.SET_SYNC])
        self.assertFalse(kbc._capabilities[KBCapabilities.FREQ_2400])
        self.assertFalse(kbc._capabilities[KBCapabilities.FREQ_900])
        self.assertFalse(kbc._capabilities[KBCapabilities.FREQ_863])
        self.assertFalse(kbc._capabilities[KBCapabilities.FREQ_868])
        self.assertFalse(kbc._capabilities[KBCapabilities.FREQ_870])
        self.assertFalse(kbc._capabilities[KBCapabilities.FREQ_915])
        self.assertFalse(kbc._capabilities[KBCapabilities.BOOT])

    def test_kbcapabilities_check(self):
        kbc=KBCapabilities()
    
        self.assertFalse(kbc.check(KBCapabilities.NONE))
        self.assertFalse(kbc.check(KBCapabilities.SNIFF))
        self.assertFalse(kbc.check(KBCapabilities.SETCHAN))
        self.assertFalse(kbc.check(KBCapabilities.INJECT))
        self.assertFalse(kbc.check(KBCapabilities.PHYJAM))
        self.assertFalse(kbc.check(KBCapabilities.SELFACK))
        self.assertFalse(kbc.check(KBCapabilities.PHYJAM_REFLEX))
        self.assertFalse(kbc.check(KBCapabilities.SET_SYNC))
        self.assertFalse(kbc.check(KBCapabilities.FREQ_2400))
        self.assertFalse(kbc.check(KBCapabilities.FREQ_900))
        self.assertFalse(kbc.check(KBCapabilities.FREQ_863))
        self.assertFalse(kbc.check(KBCapabilities.FREQ_868))
        self.assertFalse(kbc.check(KBCapabilities.FREQ_870))
        self.assertFalse(kbc.check(KBCapabilities.FREQ_915))
        self.assertFalse(kbc.check(KBCapabilities.BOOT))

    def test_kbcapabilities_getlist(self):
        kbc=KBCapabilities()
        capabilities = kbc.getlist()
    
        self.assertFalse(capabilities[KBCapabilities.NONE])
        self.assertFalse(capabilities[KBCapabilities.SNIFF])
        self.assertFalse(capabilities[KBCapabilities.SETCHAN])
        self.assertFalse(capabilities[KBCapabilities.INJECT])
        self.assertFalse(capabilities[KBCapabilities.PHYJAM])
        self.assertFalse(capabilities[KBCapabilities.SELFACK])
        self.assertFalse(capabilities[KBCapabilities.PHYJAM_REFLEX])
        self.assertFalse(capabilities[KBCapabilities.SET_SYNC])
        self.assertFalse(capabilities[KBCapabilities.FREQ_2400])
        self.assertFalse(capabilities[KBCapabilities.FREQ_900])
        self.assertFalse(capabilities[KBCapabilities.FREQ_863])
        self.assertFalse(capabilities[KBCapabilities.FREQ_868])
        self.assertFalse(capabilities[KBCapabilities.FREQ_870])
        self.assertFalse(capabilities[KBCapabilities.FREQ_915])
        self.assertFalse(capabilities[KBCapabilities.BOOT])

    def test_kbcapabilities_setcapab(self): 
        kbc=KBCapabilities()
      
        self.assertFalse(kbc.check(KBCapabilities.NONE))

        kbc.setcapab(KBCapabilities.NONE, True)

        self.assertTrue(kbc.check(KBCapabilities.NONE))

    def test_kbcapabilities_require(self):
        kbc=KBCapabilities()

        self.assertRaises(Exception, kbc.require, KBCapabilities.NONE)

    def test_kbcapabilities_is_valid_channel(self):
        kbc=KBCapabilities()
        
        #Check FREQ_863
        kbc.setcapab(KBCapabilities.FREQ_863, True)
        self.assertFalse(kbc.is_valid_channel(27, 28))
        self.assertTrue(kbc.is_valid_channel(26, 28))
        kbc.setcapab(KBCapabilities.FREQ_863, False)
        self.assertFalse(kbc.is_valid_channel(26, 28))

        #Check FREQ_868
        kbc.setcapab(KBCapabilities.FREQ_868, True)
        self.assertFalse(kbc.is_valid_channel(9, 29))
        self.assertTrue(kbc.is_valid_channel(8, 29))
        kbc.setcapab(KBCapabilities.FREQ_868, False)
        self.assertFalse(kbc.is_valid_channel(8, 29))

        #Check FREQ_870
        kbc.setcapab(KBCapabilities.FREQ_870, True)
        self.assertFalse(kbc.is_valid_channel(27, 30))
        self.assertTrue(kbc.is_valid_channel(26, 30))
        kbc.setcapab(KBCapabilities.FREQ_870, False)
        self.assertFalse(kbc.is_valid_channel(26, 30))

        #Check FREQ_915
        kbc.setcapab(KBCapabilities.FREQ_915, True)
        self.assertFalse(kbc.is_valid_channel(27, 31))
        self.assertTrue(kbc.is_valid_channel(26, 31))
        kbc.setcapab(KBCapabilities.FREQ_915, False)
        self.assertFalse(kbc.is_valid_channel(26, 31))

        kbc.setcapab(KBCapabilities.FREQ_863, True)
        kbc.setcapab(KBCapabilities.FREQ_868, True)
        kbc.setcapab(KBCapabilities.FREQ_870, True)
        kbc.setcapab(KBCapabilities.FREQ_915, True)
        self.assertFalse(kbc.is_valid_channel(1, 27))
        self.assertFalse(kbc.is_valid_channel(1, 32))
        kbc.setcapab(KBCapabilities.FREQ_863, False)
        kbc.setcapab(KBCapabilities.FREQ_868, False)
        kbc.setcapab(KBCapabilities.FREQ_870, False)
        kbc.setcapab(KBCapabilities.FREQ_915, False)

        #Check FREQ_2400
        kbc.setcapab(KBCapabilities.FREQ_2400, True)
        self.assertTrue(kbc.is_valid_channel(11))
        self.assertFalse(kbc.is_valid_channel(10))
        self.assertFalse(kbc.is_valid_channel(27))
        kbc.setcapab(KBCapabilities.FREQ_2400, False)

        #Check FREQ_900
        kbc.setcapab(KBCapabilities.FREQ_900, True)
        self.assertTrue(kbc.is_valid_channel(1))
        self.assertFalse(kbc.is_valid_channel(11))
        self.assertFalse(kbc.is_valid_channel(0))
        kbc.setcapab(KBCapabilities.FREQ_900, False)

    def test_kbcapabilities_frequency(self):
        kbc=KBCapabilities()
      
        #Check FREQ_2400
        kbc.setcapab(KBCapabilities.FREQ_2400, True)
        self.assertEqual(0, kbc.frequency(11, 1))
        self.assertEqual(2405000, kbc.frequency(11))
        self.assertEqual(2480000, kbc.frequency(26))
        kbc.setcapab(KBCapabilities.FREQ_2400, False)

        #Check FREQ_900 NOT SUPPORTED
        #kbc.setcapab(KBCapabilities.FREQ_900, True)
        #self.assertEqual(0, kbc.frequency(1))

        #Check FREQ_863
        kbc.setcapab(KBCapabilities.FREQ_863, True)
        self.assertEqual(0, kbc.frequency(27, 28))
        self.assertEqual(863250, kbc.frequency(0, 28))
        self.assertEqual(868450, kbc.frequency(26, 28))
        kbc.setcapab(KBCapabilities.FREQ_863, False)

        #Check FREQ_868
        kbc.setcapab(KBCapabilities.FREQ_868, True)
        self.assertEqual(0, kbc.frequency(9, 29))
        self.assertEqual(868650, kbc.frequency(0, 29))
        self.assertEqual(870250, kbc.frequency(8, 29))
        kbc.setcapab(KBCapabilities.FREQ_868, False)

        #Check FREQ_870
        kbc.setcapab(KBCapabilities.FREQ_870, True)
        self.assertEqual(0, kbc.frequency(27, 30))
        self.assertEqual(870250, kbc.frequency(0, 30))
        self.assertEqual(875450, kbc.frequency(26, 30))
        kbc.setcapab(KBCapabilities.FREQ_870, False)

        #Check FREQ_915
        kbc.setcapab(KBCapabilities.FREQ_915, True)
        self.assertEqual(0, kbc.frequency(27, 31))
        self.assertEqual(915350, kbc.frequency(0, 31))
        self.assertEqual(920550, kbc.frequency(26, 31))
        kbc.setcapab(KBCapabilities.FREQ_915, False)

    #TODO USB Discovery
    def test_findFromList_init(self):
        self.assertTrue(True)
    def test_findFromList_call(self):
        self.assertTrue(True)
    def test_findFromListAndBusDevId_init(self):
        self.assertTrue(True)
    def test_findFromListAndBusDevId_call(self):
        self.assertTrue(True)
    def test_devlist_usb_v1x(self):
        self.assertTrue(True)
    def test_devlist_usb_v0x(self):
        self.assertTrue(True)    
    def test_devlist(self):
        self.assertTrue(True)
    def test_getSerialDeviceString(self):
        self.assertTrue(True)
    def test_get_serial_ports(self):
        self.assertTrue(True)

    def test_isIpAddr(self):
        self.assertFalse(isIpAddr("256.256.256.256"))
        self.assertFalse(isIpAddr("1.1.1.1.1"))
        self.assertFalse(isIpAddr("1.1.1"))
        self.assertFalse(isIpAddr("not an ip address"))
        self.assertTrue(isIpAddr("1.1.1.1"))
        self.assertTrue(isIpAddr("255.255.255.255"))

        self.assertTrue(isIpAddr("2607::4"))
        self.assertTrue(isIpAddr("2607:f0d0:1002:51::4"))
        self.assertTrue(isIpAddr("2607:f0d0:1002:0051:0000:0000:0000:0004"))
        self.assertFalse(isIpAddr("gggg:gggg:gggg:gggg:gggg:gggg:gggg:gggg:gggg"))

    #TODO Device ID
    def test_iszigduino(self):
        self.assertTrue(True)
    def test_issl_nodetest(self):
        self.assertTrue(True)
    def test_issl_beehive(self):
        self.assertTrue(True)
    def test_isfreakduino(self):
        self.assertTrue(True)

    #TODO More USB
    def test_search_usb(self):
        self.assertTrue(True)
    def test_search_usb_bus_v0x(self):
        self.assertTrue(True)
    
    #TODO migrate this function to scapy hexdump? This doesn't work in Python3
    def test_hexdump(self):
        self.assertRaises(TypeError, hexdump, "test") #SHOULD NOT RAISE ERROR

    def test_randbytes(self):
        self.assertEqual(1, len(randbytes(1)))
        self.assertEqual(4, len(randbytes(4)))
        self.assertEqual(100, len(randbytes(100)))
        self.assertEqual(1000, len(randbytes(1000)))

    def test_randmac(self):
        self.assertEqual(8, len(randmac()))

    def test_makeFCS(self):
        data = b'\x01' * 20
        self.assertEqual(b'\xa7\xf7', makeFCS(data))
        data = b'\x02' * 20
        self.assertEqual(b'\x5f\xe7', makeFCS(data))

    def test_bytearray_to_bytes(self):
        b = bytes([0x01, 0x02, 0x03, 0x04])
        self.assertEqual(b, bytearray_to_bytes(b))
        
if __name__ == "__main__":
    unittest.main()


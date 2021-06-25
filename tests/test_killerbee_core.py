import unittest
import struct
import argparse
import os

from killerbee import *
from killerbee.dev_apimote import APIMOTE

class TestKillerbeeCore(unittest.TestCase):
    def test_show_dev(self):
        show_dev()
        self.assertTrue(True)

    def test_killerbee_init(self):
        devstring=os.environ['APIMOTE_DEVSTRING']

        #TODO datastore, gps, dblog
  
        #TODO usbserial throws exception
        kb = KillerBee()
        self.assertRaises(Exception, KillerBee, "/dev/null")
        
        #APIMOTE only
        kb = KillerBee(devstring)
        self.assertIsNotNone(kb.driver)
        self.assertIsNotNone(kb.driver.handle)
        self.assertIsNotNone(kb.dev)
        self.assertEqual(kb.dev, devstring)

        kb.close()

    def test_killerbee_close(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])
        self.assertIsNotNone(kb.driver.handle)
        kb.close()
        self.assertIsNone(kb.driver.handle)

    def test_killerbee_get_dev_info(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        info = kb.get_dev_info()

        self.assertEqual(os.environ['APIMOTE_DEVSTRING'], info[0])
        self.assertEqual("GoodFET Apimote v2", info[1])
        self.assertEqual("", info[2])
     
        kb.close()
     
    def test_killerbee_check_capability(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertTrue(kb.check_capability(KBCapabilities.FREQ_2400))
        self.assertTrue(kb.check_capability(KBCapabilities.SNIFF))
        self.assertTrue(kb.check_capability(KBCapabilities.SETCHAN))
        self.assertTrue(kb.check_capability(KBCapabilities.INJECT))
        self.assertTrue(kb.check_capability(KBCapabilities.PHYJAM_REFLEX))
        self.assertTrue(kb.check_capability(KBCapabilities.SET_SYNC))
        self.assertFalse(kb.check_capability(KBCapabilities.NONE))
        self.assertFalse(kb.check_capability(KBCapabilities.SELFACK))
        self.assertFalse(kb.check_capability(KBCapabilities.PHYJAM))
        self.assertFalse(kb.check_capability(KBCapabilities.FREQ_900))
        self.assertFalse(kb.check_capability(KBCapabilities.FREQ_863))
        self.assertFalse(kb.check_capability(KBCapabilities.FREQ_868))
        self.assertFalse(kb.check_capability(KBCapabilities.FREQ_870))
        self.assertFalse(kb.check_capability(KBCapabilities.FREQ_915))
        self.assertFalse(kb.check_capability(KBCapabilities.BOOT))

        kb.close()

    def test_killerbee_is_valid_channel(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertFalse(kb.is_valid_channel(27, 28))
        self.assertFalse(kb.is_valid_channel(26, 28))
        self.assertFalse(kb.is_valid_channel(9, 29))
        self.assertFalse(kb.is_valid_channel(8, 29))
        self.assertFalse(kb.is_valid_channel(8, 29))
        self.assertFalse(kb.is_valid_channel(27, 30))
        self.assertFalse(kb.is_valid_channel(26, 30))
        self.assertFalse(kb.is_valid_channel(27, 31))
        self.assertFalse(kb.is_valid_channel(26, 31))
        self.assertFalse(kb.is_valid_channel(1, 27))
        self.assertFalse(kb.is_valid_channel(1, 32))
        self.assertFalse(kb.is_valid_channel(1))
        self.assertFalse(kb.is_valid_channel(10))
        self.assertFalse(kb.is_valid_channel(27))
        self.assertTrue(kb.is_valid_channel(11))
        self.assertTrue(kb.is_valid_channel(26))

        kb.close()

    def test_killerbee_frequency(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertEqual(0, kb.frequency(11, 1))
        self.assertEqual(0, kb.frequency(1))
        self.assertEqual(0, kb.frequency(27))
        self.assertEqual(2405000, kb.frequency(11))
        self.assertEqual(2405000, kb.frequency(11, 0))
        self.assertEqual(2480000, kb.frequency(26))
        self.assertEqual(2480000, kb.frequency(26, 0))

        #TODO frequency handles none-case
        #self.assertEqual(0, kb.frequency())

        kb.close()

    def test_killerbee_get_capabilities(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])
        capabilities = kb.get_capabilities()
    
        self.assertTrue(capabilities[KBCapabilities.SNIFF])
        self.assertTrue(capabilities[KBCapabilities.SETCHAN])
        self.assertTrue(capabilities[KBCapabilities.INJECT])
        self.assertTrue(capabilities[KBCapabilities.PHYJAM_REFLEX])
        self.assertTrue(capabilities[KBCapabilities.SET_SYNC])
        self.assertTrue(capabilities[KBCapabilities.FREQ_2400])
        self.assertFalse(capabilities[KBCapabilities.NONE])
        self.assertFalse(capabilities[KBCapabilities.PHYJAM])
        self.assertFalse(capabilities[KBCapabilities.SELFACK])
        self.assertFalse(capabilities[KBCapabilities.FREQ_900])
        self.assertFalse(capabilities[KBCapabilities.FREQ_863])
        self.assertFalse(capabilities[KBCapabilities.FREQ_868])
        self.assertFalse(capabilities[KBCapabilities.FREQ_870])
        self.assertFalse(capabilities[KBCapabilities.FREQ_915])
        self.assertFalse(capabilities[KBCapabilities.BOOT])

    def test_killerbee_enter_bootloader(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(AttributeError, kb.enter_bootloader)
  
        kb.close()

    def test_killerbee_get_bootloader_version(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(AttributeError, kb.get_bootloader_version)
  
        kb.close()

    def test_killerbee_get_bootloader_signature(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(AttributeError, kb.get_bootloader_signature)
  
        kb.close()

    def test_killerbee_bootloader_sign_on(self): 
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(AttributeError, kb.bootloader_sign_on)
  
        kb.close()

    def test_killerbee_bootloader_start_application(self): 
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(AttributeError, kb.bootloader_start_application)
  
        kb.close()

    def test_killerbee_sniffer_on_off(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])
        kb.sniffer_on()

        kb.sniffer_off()

        kb.close()

        self.assertTrue(True)

    def test_killerbee_channel(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertEqual(None, kb.channel)

        kb.set_channel(15)

        self.assertEqual(15, kb.channel)

        kb.close()
        
    def test_killerbee_page(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertEqual(0, kb.page)

        #kb.set_channel(15, 5)

        #self.assertEqual(5, kb.page)
        
        kb.close() 

    def test_killerbee_set_channel(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertEqual(None, kb.channel)

        kb.set_channel(15)

        self.assertEqual(15, kb.channel)

        self.assertRaises(ValueError, kb.set_channel, 15, 5)

        kb.close()

    def test_killerbee_inject(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(Exception, kb.inject, b'')

        packet = struct.pack("<126s", (b'\x01'*126))
        self.assertRaises(Exception, kb.inject, packet)

        packet = struct.pack("<100s", (b'\x01'*100))
        kb.inject(packet)

        kb.close()

    def test_killerbee_pnext(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        kb.pnext()

        self.assertTrue(True)
  
        kb.close()

    def test_killerbee_jammer_on_off(self):
        kb = KillerBee(os.environ['APIMOTE_DEVSTRING'])

        kb.jammer_on()

        self.assertRaises(Exception, kb.jammer_off)

        kb.close()

if __name__ == "__main__":
    unittest.main()


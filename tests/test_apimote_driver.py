import unittest
import struct
import argparse
import os

from killerbee.dev_apimote import APIMOTE
from killerbee.kbutils import KBCapabilities

class TestApimoteDriver(unittest.TestCase):
    def test_init(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])
    
        self.assertEqual("apimote2", os.environ['platform'])
        self.assertEqual("apimote2", os.environ['board'])
        self.assertIsNotNone(driver.handle)
        
        driver.close()

    def test_close(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])
        self.assertIsNotNone(driver.handle)
        driver.close()
        self.assertIsNone(driver.handle)

    def test_get_capabilities(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])

        capabilities = driver.get_capabilities()
        self.assertTrue(capabilities[KBCapabilities.FREQ_2400])
        self.assertTrue(capabilities[KBCapabilities.SNIFF])
        self.assertTrue(capabilities[KBCapabilities.SETCHAN])
        self.assertTrue(capabilities[KBCapabilities.INJECT])
        self.assertTrue(capabilities[KBCapabilities.PHYJAM_REFLEX])
        self.assertTrue(capabilities[KBCapabilities.SET_SYNC])
        self.assertFalse(capabilities[KBCapabilities.NONE])
        self.assertFalse(capabilities[KBCapabilities.SELFACK])
        self.assertFalse(capabilities[KBCapabilities.PHYJAM])
        self.assertFalse(capabilities[KBCapabilities.FREQ_900])
        self.assertFalse(capabilities[KBCapabilities.FREQ_863])
        self.assertFalse(capabilities[KBCapabilities.FREQ_868])
        self.assertFalse(capabilities[KBCapabilities.FREQ_870])
        self.assertFalse(capabilities[KBCapabilities.FREQ_915])
        self.assertFalse(capabilities[KBCapabilities.BOOT])

        driver.close()

    def test_get_dev_info(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])
   
        info = driver.get_dev_info()
        self.assertEqual(os.environ['APIMOTE_DEVSTRING'], info[0])
        self.assertEqual("GoodFET Apimote v2", info[1])
        self.assertEqual("", info[2])
        
        driver.close()

    def test_check_capabilities(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])

        self.assertTrue(driver.check_capability(KBCapabilities.FREQ_2400))
        self.assertTrue(driver.check_capability(KBCapabilities.SNIFF))
        self.assertTrue(driver.check_capability(KBCapabilities.SETCHAN))
        self.assertTrue(driver.check_capability(KBCapabilities.INJECT))
        self.assertTrue(driver.check_capability(KBCapabilities.PHYJAM_REFLEX))
        self.assertTrue(driver.check_capability(KBCapabilities.SET_SYNC))
        self.assertFalse(driver.check_capability(KBCapabilities.NONE))
        self.assertFalse(driver.check_capability(KBCapabilities.SELFACK))
        self.assertFalse(driver.check_capability(KBCapabilities.PHYJAM))
        self.assertFalse(driver.check_capability(KBCapabilities.FREQ_900))
        self.assertFalse(driver.check_capability(KBCapabilities.FREQ_863))
        self.assertFalse(driver.check_capability(KBCapabilities.FREQ_868))
        self.assertFalse(driver.check_capability(KBCapabilities.FREQ_870))
        self.assertFalse(driver.check_capability(KBCapabilities.FREQ_915))
        self.assertFalse(driver.check_capability(KBCapabilities.BOOT))

        driver.close()

    def test_sniffer_on_off(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])
        driver.sniffer_on()

        driver.sniffer_off()

        driver.close()
        self.assertTrue(True)

    def test_set_channel(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(Exception, driver.set_channel, 1)
        self.assertRaises(Exception, driver.set_channel, 27)

        driver.set_channel(11)
        self.assertEqual(driver._channel, 11)

        driver.set_channel(15)
        self.assertEqual(driver._channel, 15)

        driver.set_channel(26)
        self.assertEqual(driver._channel, 26)

        driver.close()

    def test_inject(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(Exception, driver.inject, b'')
        
        packet = struct.pack("<126s", (b'\x01' * 126))         
        self.assertRaises(Exception, driver.inject, packet)

        packet = struct.pack("<100s", (b'\x01' * 100))
        driver.inject(packet)

        driver.close()

    def test_pnext(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])
     
        driver.set_channel(25) 
        driver.pnext()

        self.assertTrue(True) 

    def test_ping(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(Exception, driver.ping)

    def test_jammer_on_off(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])
     
        #driver.jammer_on(None, 0, 'reflexive')
        #driver.jammer_on(None, 0, 'constant')
        #driver.jammer_on(11, 0, 'constant')
        self.assertRaises(Exception, driver.jammer_on)
        self.assertRaises(Exception, driver.jammer_on, None, 0, 'none')
        self.assertRaises(Exception, driver.jammer_on, None, 0, None)
        self.assertRaises(Exception, driver.jammer_off)

        driver.close()

    def test_set_sync(self):
        driver=APIMOTE(os.environ['APIMOTE_DEVSTRING'])

        self.assertRaises(Exception, driver.set_sync, 0x0123456)

        driver.set_sync(0x1234)

        driver.close()
        

if __name__ == "__main__":
    unittest.main()


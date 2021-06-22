import unittest
import struct
import argparse
import os

from zigbee_crypt import * 

class TestZigbeeCrypt(unittest.TestCase):
    def test_encrypt_ccm(self):
        key = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f'
        nonce = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c'
        pt_data = b'\x01\x02\x03\x04'
        zigbee_data = b'\x00\x00\x00\x00'
        mic_len = 4 
       
        (enc_data, mic) = encrypt_ccm(key, nonce, mic_len, pt_data, zigbee_data)

        self.assertEqual(b'\x17\x36\xb7\x8c', enc_data)
        self.assertEqual(b'\xfc\xe0\xce\x86', mic)

    def test_decrypt_cm(self):
        key = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f'
        nonce = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c'
        enc_data = b'\x17\x36\xb7\x8c'
        mic = b'\xfc\xe0\xce\x86'
        zigbee_data = b'\x00\x00\x00\x00'
     
        (pt_data, mic_check) = decrypt_ccm(key, nonce, mic, enc_data, zigbee_data)
 
        self.assertEqual(b'\x01\x02\x03\x04', pt_data) 
        self.assertTrue(mic_check)

    def test_sec_key_hash(self):
        key = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f'
        key_hash = sec_key_hash(key, b'\x00') 
        self.assertEqual(b'\xd2\x28\x9c\x6f\xeb\xfe\xdc\xb8\x91\xda\x27\xdc\xd0\xb6\x88\x5d', key_hash)
        
if __name__ == "__main__":
    unittest.main()


import unittest
import struct
import argparse
import os
from killerbee.daintree import * 

class TestDaintree(unittest.TestCase):
    def test_daintreedumper_init(self):
        path_to_file = "./tests/fixtures/test_dt_dump.dcf"
        dt = DainTreeDumper(path_to_file)
        dt.close()

        with open(path_to_file) as f:
          self.assertTrue('#Format=4' in f.readline())
          self.assertTrue('# SNA v3.0.0.7 SUS:' in f.readline())

    def test_daintreedumper_pcap_dump(self):
        self.assertTrue(True)

    def test_daintreedumper_pwrite(self):
        self.assertTrue(True)

    def test_daintreedumper_close(self):
        path_to_file = "./tests/fixtures/test_dt_dump.dcf"
        dt = DainTreeDumper(path_to_file)
        dt.close()

        self.assertTrue(True)

    def test_daintreereader_init(self):
        path_to_file = "./tests/fixtures/test_dt_read.dcf"
        dt = DainTreeReader(path_to_file)
        dt.close()

        self.assertTrue(True)
        self.assertRaises(Exception, DainTreeReader, "/dev/null")

    def test_daintreereader_close(self):
        path_to_file = "./tests/fixtures/test_dt_read.dcf"
        dt = DainTreeReader(path_to_file)
        dt.close()

        self.assertTrue(True)

    def test_daintreereader_pnext(self):
        path_to_file = "./tests/fixtures/test_dt_read.dcf"
        dt = DainTreeReader(path_to_file)
        out = dt.pnext()
        self.assertTrue(True)
        dt.close()
        self.assertTrue(True)

if __name__ == "__main__":
    unittest.main()


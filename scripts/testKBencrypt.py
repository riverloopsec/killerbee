#! /usr/bin/env python

# Import logging to suppress Warning messages
import logging
logging.getLogger("scapy.runtime").setLevel(logging.ERROR)

try:
	from scapy.all import *
except ImportError:
	print 'This Requires Scapy To Be Installed.'
	from sys import exit
	exit(-1)

from killerbee.scapy_extensions import *

del hexdump
import os, sys
from glob import glob
###############################

###############################
# Processing Functions
###############################
# Defaults
indent      = "    "
DEBUG       = False
SHOW_RAW    = False
#zb_file     = None
zb_files    = []
find_key    = False
network_key = None
cert_key    = None

def usage():
    print "%s Usage"%sys.argv[0]
    print "    -h: help"
    print "    -f <filename>: capture file with zigbee packets."
    print "    -d <directory name>: directory containing capture files with zigbee packets."
    print "    -k <network_key>: Network Key in ASCII format. Will be converted for use."
    print "    -D: Turn on debugging."
    sys.exit()

def detect_encryption(pkt):
    '''detect_entryption: Does this packet have encrypted information? Return: True or False'''
    if not pkt.haslayer(ZigbeeSecurityHeader) or not pkt.haslayer(ZigbeeNWK):
        return False
    return True

###############################

if __name__ == '__main__':

    # Process options
    ops = ['-f','-d','-k','-D','-h']

    while len(sys.argv) > 1:
        op = sys.argv.pop(1)
        if op == '-f':
            zb_files = [sys.argv.pop(1)]
        if op == '-d':
            dir_name = sys.argv.pop(1)
            zb_files = glob(os.path.abspath(os.path.expanduser(os.path.expandvars(dir_name))) + '/*.pcap')
        if op == '-k':
            network_key = sys.argv.pop(1).decode('hex')
        if op == '-D':
            DEBUG = True
        if op == '-h':
            usage()
        if op not in ops:
            print "Unknown option:",op
            usage()

    # Test for user input
    if not zb_files: usage()

    if DEBUG: print "\nProcessing files:",zb_files,"\n"
    for zb_file in zb_files:
        if DEBUG: print "\nProcessing file:",zb_file,"\n"
        data = kbrdpcap(zb_file)
        num_pkts = len(data)

        for e in range(num_pkts):
            if detect_encryption(data[e]):
                if network_key:
                    dec_data = kbdecrypt(data[e],network_key, 1)
                    re_enc_data = kbencrypt(data[e], dec_data, network_key, 1)
                    re_de_data = kbdecrypt(re_enc_data, network_key, 1)

                    print "\t    ORIGINAL Packet: " + str(data[e]).encode('hex')
                    print "\tRE-Encrypted Packet: " + str(re_enc_data).encode('hex')
                    print "\t   Decrypted Packet: " + dec_data.encode('hex')
                    print "\tRE-Decrypted Packet: " + str(re_de_data).encode('hex')

                else:
                    print indent*3 + "Has Encrypted Data, no network key provided."
        print ""
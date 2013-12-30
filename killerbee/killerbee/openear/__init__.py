#!/usr/bin/env python

# ZBOpenEar
# rmspeers 2010
# ZigBee/802.15.4 Many Channel Listener

import sys
import string
import socket
import struct
#import bitstring

from killerbee import *
from capture import *

# Command line main function
if __name__=='__main__':
    arg_verbose = False
    # parse command line options
    while len(sys.argv) > 1:
        op = sys.argv.pop(1)
        if op == '-v':
            arg_verbose = True

    # try-except block to catch keyboard interrupt.
    try:
        kbdev_info = kb_dev_list()
        channel = 11
        for i in range(0, len(kbdev_info)):
            print 'Found device at %s: \'%s\'' % (kbdev_info[i][0], kbdev_info[i][1])
            if channel <= 26:
                print '\tAssigning to channel %d.' % channel
                startCapture(kbdev_info[i][0], channel)
                channel += 1
    except KeyboardInterrupt:
        print 'Shutting down'

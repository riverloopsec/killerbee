#!/usr/bin/env python
from killerbee import *

print "t1: get_dev_info then inject a packet 5 times on channel 26 with .5us delay"
kb = KillerBee()
print kb.get_dev_info()
packet = "\x69\x88\x00\x07\x20\xef\xbe\x20\x25\x06\x0a\x00\x00\x00\xf1\xc7\xfa\x14\xe9\x66\xeb\x89\x33"
kb.inject(packet, channel=26, count=5, delay=0.5)
kb.close()

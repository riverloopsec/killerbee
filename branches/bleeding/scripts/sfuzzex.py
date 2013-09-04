# This is a simple example of integrating Sulley for packet mutation with
# KillerBee as the delivery mechanism.  Sulley is a lot more sophisticated and
# can do a lot more interesting mutation than this simple example, but it
# should be enough to get you going.  I'm still working on a reliable "ping"
# function to measure the availability of the target.  -Josh

import sys
#TODO system path MUST be changed to your sully install path
sys.path.append("/home/jwright/fuzzing/sulley")

from killerbee import *
from sulley import *

dstpan = "\xff\xff"
dst = "\xff\xff"
src = "\x00\x0d\x6f\x04\x00\x00\x00\x50"

s_initialize("MAC Command Mutation")
s_static("\x43\xc8\x00") # FC + Seq#
s_static(dstpan[::-1]) # Addresses are specified in normal order, then reversed
s_static(dst[::-1])
s_static(src[::-1])
s_byte(0, full_range=True)

kb = KillerBee()
kb.set_channel(26)

while s_mutate():
    kb.inject(s_render())
    print hexdump(s_render())


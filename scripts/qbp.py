#!/usr/bin/env
from killerbee import *
import sys

print "QuahogCon Badge Pwn (qbp) - jwright@willhackforsushi.com"

if len(sys.argv) == 1:
    print """
Usage: qbp [mode]

Mode is one of:

    uber    - Become uber (there can be only one)
    cleric  - Become a cleric
    mussel  - Become a mussel
    zombie  - Become a zombie
    dead    - Become dead
    human   - Become a human

Packets get sent 5 times, for good measure.
"""
else:
    
    modes = {
    	"cleric" : "\xFF\x00\x63\x03\x65\xd9\x3d\x91\xf5\x29\x8d\xe1\x45\xb9\x1d\x71\x00\x2c\x00",
    	"uber" : "\xFF\x00\x63\x06\x65\xd9\x3d\x91\xf5\x29\x8d\xe1\x45\xb9\x1d\x71\x00\x2c\x00",
    	"zombie" : "\xFF\x00\x63\x02\x65\xd9\x3d\x91\xf5\x29\x8d\xe1\x45\xb9\x1d\x71\x00\x2c\x00",
    	"dead" : "\xFF\x00\x63\x04\x65\xd9\x3d\x91\xf5\x29\x8d\xe1\x45\xb9\x1d\x71\x00\x2c\x00",
    	"mussel" : "\xFF\x00\x63\x05\x65\xd9\x3d\x91\xf5\x29\x8d\xe1\x45\xb9\x1d\x71\x00\x2c\x00",
    	"human" : "\xFF\x00\x63\x01\x65\xd9\x3d\x91\xf5\x29\x8d\xe1\x45\xb9\x1d\x71\x00\x2c\x00" }
    
    kb = KillerBee()
    kb.set_channel(26)
    try:
        for i in xrange(0,5):
            kb.inject(modes[sys.argv[1]])
            time.sleep(.2)
    except:
        print "Something got messed up, please check your mode designation"

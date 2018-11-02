#!/usr/bin/env python

import sys
import signal
import argparse
from killerbee import *
from killerbee.dev_sewio import SEWIO


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('-i', '--iface', '--dev', action='store', dest='devstring')
#parser.add_argument('-g', '--gps', '--ignore', action='append', dest='ignore')
parser.add_argument('-w', '--pcapfile', action='store')
parser.add_argument('-W', '--dsnafile', action='store')
parser.add_argument('-p', '--ppi', action='store_true')
parser.add_argument('-c', '-f', '--channel', action='store', type=int, default=None)
parser.add_argument('-n', '--count', action='store', type=int, default=-1)
parser.add_argument('-D', action='store_true', dest='showdev')
args = parser.parse_args()

if args.showdev:
    show_dev(include=[args.devstring])
    sys.exit(0)

kb = KillerBee(device=args.devstring)
rf_freq_mhz = kb.get_frequency(args.channel)
print rf_freq_mhz
#!/bin/sh

# Flash the gf-zigduino.hex compiled firmware image
# to a Zigduino hardware platform.
# REQUIRES avrdude tool (ex sudo apt-get install avrdude)

if [ -n "$1" ] && [ -c "$1" ]; then
    avrdude -V -F -c stk500v1 -p m128rfa1 -b 57600 -P $1 -U flash:w:gf-zigduino.hex
else
    echo "$0 requires path to device\n e.g. $0 /dev/ttyUSB0"
fi

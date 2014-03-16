#!/bin/sh

FW_FILE='apimotev4_gf.hex'

echo "Flashing $FW_FILE to the connected USB serial device."

board=apimote3 ./goodfet.bsl --speed=38400 -e -p $FW_FILE

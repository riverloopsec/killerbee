#!/bin/sh

FW_FILE='apimotev4_gf.hex'

echo "Flashing $FW_FILE to the connected USB serial device."

board=apimote3 ./goodfet.bsl -e -p $FW_FILE

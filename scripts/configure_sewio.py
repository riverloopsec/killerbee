#!/usr/bin/env python

'''
Configure the Sewio Open Sniffer.
'''

import argparse
import subprocess
from urllib.request import urlopen
import re

def getFirmwareVersion(ip):
    try:
        html = urlopen("http://{0}/".format(ip))
        data = html.read()
        # First try for the "old" web UI parsing:
        fw = re.search(b'Firmware version ([0-9.]+)', data)
        if fw is not None:
            return fw.group(1)
        else:
            # Detect using the new method, credit to the opensniffer python code
            # Find index of slave IP address
            index = data.find(b'ip')
            # Find index of parenthesis
            indexEnd = data.find(b')', index) - 1
            # Find index of comma before parenthesis
            indexBeg = data[:indexEnd].rindex(b',') + 1
            # Parse FW version out
            fw = data[indexBeg:indexEnd]
            if fw is not None:
                return fw.decode('UTF-8')
    except Exception as e:
        print("Unable to connect to the sniffer. You may need to manually configure the device and host interface, or check your parameters to this script.")
        print(e)
    return None

def connect(iface, hostip, sewioip):
    test = subprocess.check_output(['sudo', 'id'], stderr=subprocess.STDOUT)
    if 'root' not in str(test):
        print("Unable to get sudo access, which is needed to configure network interfaces.")
        return False
    # Bring up the interface with the right IP
    subprocess.check_output(['sudo', 'ifconfig', iface, hostip, 'up'])
    # And then get network manager to shut up
    #TODO what happens on non-network manager machines here?!!!
    res = subprocess.check_output(['nmcli', 'dev', 'status'], stderr=subprocess.STDOUT)
    res = [l for l in str(res).split('\n') if iface in l]
    if len(str(res)) == 1 and 'disconnected' not in str(res[0]):
        subprocess.check_output(['sudo', 'nmcli', 'dev', 'disconnect', 'iface', iface])
    fwVer = getFirmwareVersion(sewioip)
    if fwVer is not None:
        print("Connected to Sewio sniffer, firmware version {0}.".format(fwVer))
        return True
    else:
        print("Unable to connect to the sniffer. You may need to manually configure the device and host interface, or check your parameters to this script.")
    return False
    
def disconnect(iface, hostip, sewioip):
    print("The easiest way to disconnect on most Linux variants is to unplug the network cable.")
    #TODO

if __name__=='__main__':
    parser = argparse.ArgumentParser(description='Configure the Sewio Open Sniffer for use with KillerBee.')
    parser.add_argument('--snifferip', default="10.10.10.2")
    parser.add_argument('--hostip', default="10.10.10.1")
    parser.add_argument('--iface', default="eth0")
    parser.add_argument('command', help=\
"""The commands `connect` (after you plug in the sewio to your computer's ethernet port)
or `disconnect` (when you're done using the sniffer) are supported.""")
    args = parser.parse_args()
    if args.command == 'connect': connect(args.iface, args.hostip, args.snifferip)
    elif args.command == 'disconnect': disconnect(args.iface, args.hostip, args.snifferip)
    else: print("Unsupported command {1}.".format(args.command))

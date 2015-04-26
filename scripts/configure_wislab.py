#!/usr/bin/env python

'''
Configure the Wislab 802.15.4 sniffer.
'''

import argparse
import subprocess
import urllib2
import re

def getFirmwareVersion(ip):
    try:
        html = urllib2.urlopen("http://{0}/".format(ip))
        fw = re.search(r'Firmware version ([0-9.]+)', html.read())
        if fw is not None:
            return fw.group(1)
    except Exception as e:
        print("Unable to connect to the sniffer. You may need to manually configure the device and host interface, or check your parameters to this script.")
        print(e)
    return None

def connect(iface, hostip, wislabip):
    test = subprocess.check_output(['sudo', 'id'], stderr=subprocess.STDOUT)
    if 'root' not in test:
        print("Unable to get sudo access, which is needed to configure network interfaces.")
        return False
    # Bring up the interface with the right IP
    subprocess.call(['sudo', 'ifconfig', iface, hostip, 'up'])
    # And then get network manager to shut up
    #TODO what happens on non-network manager machines here?!!!
    res = subprocess.check_output(['nmcli', 'dev', 'status'])
    res = filter(lambda l: iface in l, res.split('\n'))
    if len(res) == 1 and 'disconnected' not in res[0]:
        subprocess.call(['sudo', 'nmcli', 'dev', 'disconnect', 'iface', iface])
    fwVer = getFirmwareVersion(wislabip)
    if fwVer is not None:
        print("Connected to Wislab sniffer, firmware version {0}.".format(fwVer))
        return True
    else:
        print("Unable to connect to the sniffer. You may need to manually configure the device and host interface, or check your parameters to this script.")
    return False
    
def disconnect(iface, hostip, wislabip):
    print("The easiest way to disconnect on most Linux variants is to unplug the network cable.")
    #TODO

if __name__=='__main__':
    parser = argparse.ArgumentParser(description='Configure the Wislab 802.15.4 sniffer for use with KillerBee.')
    parser.add_argument('--snifferip', default="10.10.10.2")
    parser.add_argument('--hostip', default="10.10.10.1")
    parser.add_argument('--iface', default="eth0")
    parser.add_argument('command', help=\
"""The commands `connect` (after you plug in the Wislab to your computer's ethernet port)
or `disconnect` (when you're done using the sniffer) are supported.""")
    args = parser.parse_args()
    if args.command == 'connect': connect(args.iface, args.hostip, args.snifferip)
    elif args.command == 'disconnect': disconnect(args.iface, args.hostip, args.snifferip)
    else: print("Unsupported command {1}.".format(args.command))

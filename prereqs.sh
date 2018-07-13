#!/bin/bash

# install prerequisites for killerbee on linux

apt install -y git python-gtk2 python-cairo python-usb python-crypto python-serial python-dev libgcrypt-dev;
pip install rangeparser;
git clone https://github.com/secdev/scapy;
cd scapy;
python setup.py install;
# clone my fork for now
git clone https://github.com/jbisterfeldt/killerbee;
cd killerbee;
python setup.py install
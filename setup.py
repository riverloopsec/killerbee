#NOTE: See the README file for a list of dependencies to install.
from __future__ import print_function  # needed for py2.7 to use print() as called below

try:
    from setuptools import setup, Extension
except ImportError:
    print("No setuptools found, attempting to use distutils instead.")
    from distutils.core import setup, Extension
import sys


err = []
warn = []

# We have made gtk, cairo, scapy-com into optional libraries
try:
    import gtk
except ImportError:
    warn.append(("gtk", "python-gtk2", ""))
    # TODO: Add pip name

try:
    import cairo
except ImportError:
    warn.append(("cairo", "python-cairo", ""))
    # TODO: Add pip name

try:
    import Crypto
except ImportError:
    err.append(("crypto", "python-crypto", "pycrypto"))

# Ensure we have either pyUSB 0.x or pyUSB 1.x, but we now
#  prefer pyUSB 1.x moving forward. Support for 0.x may be deprecated.
try:
    import usb
except ImportError:
    err.append(("usb", "python-usb", "pyusb"))

try:
    import usb.core
    #print("Warning: You are using pyUSB 1.x, support is in beta.")
except ImportError:
    warn.append(("NOTE: You are using pyUSB 0.x. Consider upgrading to pyUSB 1.x.", "", ""))
    # TODO: Add apt/pip names

try:
    import serial
except ImportError:
    err.append(("serial", "python-serial", "pyserial"))

# Dot15d4 is a dep of some of the newer tools
try:
    from scapy.all import Dot15d4
except ImportError:
    warn.append(("scapy-com with dot15d4.py", "", ""))


if len(err) > 0:
    print("""
Library requirements were not met.
Install the following libraries, then re-run the setup script.
\n\t{}\n\n
apt-get install {}
pip install {}""".format("\t".join(map(lambda x: x[0], err)),
                         " ".join(map(lambda x: x[1], err)),
                         " ".join(map(lambda x: x[2], err))), file=sys.stderr)
    sys.exit(1)

if len(warn) > 0:
    print("""
Library recommendations were not met.
For full support, install the following libraries, then re-run the setup script.
\n\t{}\n\n
apt-get install {}
pip install {}""".format("\t".join(map(lambda x: x[0], warn)),
                         " ".join(map(lambda x: x[1], warn)),
                         " ".join(map(lambda x: x[2], warn))), file=sys.stderr)
    # TODO: Offer the user to type y/n to continue or cancel at this point

zigbee_crypt = Extension('zigbee_crypt',
                    sources = ['zigbee_crypt/zigbee_crypt.c'],
                    libraries = ['gcrypt'],
                    include_dirs = ['/usr/local/include', '/usr/include', '/sw/include/', 'zigbee_crypt'],
                    library_dirs = ['/usr/local/lib', '/usr/lib','/sw/var/lib/']
                    )

setup  (name        = 'killerbee',
        version     = '3.0.0',
        description = 'ZigBee and IEEE 802.15.4 Attack Framework and Tools',
        author = 'Joshua Wright, Ryan Speers',
        author_email = 'jwright@willhackforsushi.com, ryan@riverloopsecurity.com',
        license   = 'LICENSE.txt',
        packages  = ['killerbee', 'killerbee.openear', 'killerbee.zbwardrive'],
        requires = ['Crypto', 'usb', 'gtk', 'cairo'], # Not causing setup to fail, not sure why
        scripts = ['tools/zbdump', 'tools/zbgoodfind', 'tools/zbid', 'tools/zbreplay', 
                   'tools/zbconvert', 'tools/zbdsniff', 'tools/zbstumbler', 'tools/zbassocflood', 
                   'tools/zbfind', 'tools/zbscapy', 'tools/zbwireshark', 'tools/zbkey', 
                   'tools/zbwardrive', 'tools/zbopenear', 'tools/zbfakebeacon',
                   'tools/zborphannotify', 'tools/zbpanidconflictflood', 'tools/zbrealign'],
        install_requires=['pyserial>=2.0', 'pyusb', 'crypto'],
        ext_modules = [ zigbee_crypt ],
        )


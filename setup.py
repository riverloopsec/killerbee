#NOTE: See the README file for a list of dependencies to install.

try:
    from setuptools import setup, Extension
except ImportError:
    print("No setuptools found, attempting to use distutils instead.")
    from distutils.core import setup, Extension
import sys

err = ""

#TODO consider making gtk, cairo into optional libraries
try:
    import gtk
except ImportError:
    err += "gtk (apt-get install python-gtk2)\n"

try:
    import cairo
except ImportError:
    err += "cairo (apt-get install python-cairo)\n"

try:
    import Crypto
except ImportError:
    err += "crypto (apt-get install python-crypto)\n"

# Ensure we have either pyUSB 0.x or pyUSB 1.x, but we now
#  prefer pyUSB 1.x moving forward. Support for 0.x may be deprecated.
try:
    import usb
except ImportError:
    err += "usb (apt-get install python-usb)\n"

try:
    import usb.core
    #print("Warning: You are using pyUSB 1.x, support is in beta.")
except ImportError:
    print("Note: You are using pyUSB 0.x. Consider upgrading to pyUSB 1.x.")

try:
    import serial
except ImportError:
    err += "serial (apt-get install python-serial)\n"

if err != "":
    print >>sys.stderr, """
Library requirements not met.  Install the following libraries, then re-run
the setup script.

    """, err
    sys.exit(1)

zigbee_crypt = Extension('zigbee_crypt',
                    sources = ['zigbee_crypt/zigbee_crypt.c'],
                    libraries = ['gcrypt'],
                    include_dirs = ['/usr/local/include', '/usr/include', '/sw/include/', 'zigbee_crypt'],
                    library_dirs = ['/usr/local/lib', '/usr/lib','/sw/var/lib/']
                    )
    

setup  (name        = 'killerbee',
        version     = '2.5.0',
        description = 'ZigBee and IEEE 802.15.4 Attack Framework and Tools',
        author = 'Joshua Wright, Ryan Speers, Ricky Melgares',
        author_email = 'jwright@willhackforsushi.com, ryan@riverloopsecurity.com',
        license   = 'LICENSE.txt',
        packages  = ['killerbee', 'killerbee.openear', 'killerbee.zbwardrive'],
        requires = ['Crypto', 'usb', 'gtk', 'cairo'], # Not causing setup to fail, not sure why
        scripts = ['tools/zbdump', 'tools/zbgoodfind', 'tools/zbid', 'tools/zbreplay', 
                   'tools/zbconvert', 'tools/zbdsniff', 'tools/zbstumbler', 'tools/zbassocflood', 
                   'tools/zbfind', 'tools/zbscapy', 'tools/zbwireshark', 'tools/zbkey', 
                    'tools/zbwardrive', 'tools/zbopenear', 'tools/zbfakebeacon', 
                    'tools/orphannotify', 'tools/zbpanidconflictflood', 'tools/zbrealign'],
        install_requires=['pyserial', 'pyusb', 'crypto'],
        ext_modules = [ zigbee_crypt ],
        )


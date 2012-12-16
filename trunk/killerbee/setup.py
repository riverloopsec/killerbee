#NOTE: See the README file for a list of dependencies to install.

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

try:
    import usb
except ImportError:
    err += "usb (apt-get install python-usb)\n"

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
        version     = '1.5',
        description = 'ZigBee and IEEE 802.15.4 Attack Framework and Tools',
        author = 'Joshua Wright, Ryan Speers, Ricky Melgares',
        author_email = 'jwright@willhackforsushi.com, ryan@rmspeers.com',
        packages  = ['killerbee'],
        requires = ['Crypto', 'usb', 'gtk', 'cairo'], # Not causing setup to fail, not sure why
        scripts = ['tools/zbdump', 'tools/zbgoodfind', 'tools/zbid', 'tools/zbreplay', 'tools/zbconvert', 'tools/zbdsniff', 'tools/zbstumbler', 'tools/zbassocflood', 'tools/zbfind', 'tools/zbscapy', 'tools/zbwireshark'],
        ext_modules = [ zigbee_crypt ],
        )

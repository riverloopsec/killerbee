# NOTE: See the README file for a list of dependencies to install.

from __future__ import print_function
import sys

try:
    from setuptools import setup, Extension
except ImportError:
    print("No setuptools found, attempting to use distutils instead.")
    from distutils.core import setup, Extension

err = []
warn = []
apt_get_pkgs = []
pip_pkgs = []

#Check pyUSB 1.x dependency present.
#Support for 0.x is deprecated and being removed.
try:
    import usb
except ImportError:
    err.append("usb")
    apt_get_pkgs.append("python-usb")

try:
    import usb.core
except ImportError:
    err.append("You are using pyUSB 0.x. Upgrade to pyUSB 1.x.")

# TODO: Ideally we would detect missing python-dev and libgcrypt-dev to give better errors.

# Dot15d4 is a dep of some of the newer tools
try:
    from scapy.all import Dot15d4
except ImportError:
    warn.append("Scapy (with dot15d4 layer)")
    pip_pkgs.append("scapy")

if len(err) > 0:
    print("""
Library requirements not met.  Install the following libraries, then re-run the setup script.

{}\n""".format('\n'.join(err)), file=sys.stderr)

if len(warn) > 0:
    print("""
Library recommendations not met. For full support, install the following libraries, then re-run the setup script.

{}\n""".format('\n'.join(warn)), file=sys.stderr)

if len(apt_get_pkgs) > 0 or len(pip_pkgs) > 0:
    print("The following commands should install these dependencies on Ubuntu, and can be adapted for other OSs:", file=sys.stderr)
    if len(apt_get_pkgs) > 0:
        print("\tsudo apt-get install -y {}".format(' '.join(apt_get_pkgs)), file=sys.stderr)
    if len(pip_pkgs) > 0:
        print("\tpip3 install {}".format(' '.join(pip_pkgs)), file=sys.stderr)

if len(err) > 0:
    sys.exit(1)

zigbee_crypt = Extension('zigbee_crypt',
                         sources = ['zigbee_crypt/zigbee_crypt.c'],
                         libraries = ['gcrypt'],
                         include_dirs = ['/usr/local/include', '/usr/include', '/sw/include/', 'zigbee_crypt'],
                         library_dirs = ['/usr/local/lib', '/usr/lib','/sw/var/lib/']
                         )

setup(name        = 'killerbee',
      version     = '3.0.0-beta.2',
      description = 'ZigBee and IEEE 802.15.4 Attack Framework and Tools',
      author = 'Joshua Wright, Ryan Speers',
      author_email = 'jwright@willhackforsushi.com, ryan@riverloopsecurity.com',
      license   = 'LICENSE.txt',
      packages  = ['killerbee'],
      scripts = ['tools/zbdump', 'tools/zbgoodfind', 'tools/zbid', 'tools/zbreplay',
                 'tools/zbconvert', 'tools/zbdsniff', 'tools/zbstumbler', 'tools/zbassocflood',
                 'tools/zbscapy', 'tools/zbwireshark', 'tools/zbkey',
                 'tools/zbwardrive', 'tools/zbopenear', 'tools/zbfakebeacon',
                 'tools/zborphannotify', 'tools/zbpanidconflictflood', 'tools/zbrealign', 'tools/zbcat',
                 'tools/zbjammer', 'tools/kbbootloader'],
      install_requires=['pyserial>=2.0', 'pyusb', 'pycryptodome', 'rangeparser', 'scapy'],
      # NOTE: pygtk doesn't install via distutils on non-Windows hosts
      ext_modules = [zigbee_crypt],
      )

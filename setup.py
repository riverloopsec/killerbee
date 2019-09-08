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

# We have made gtk, cairo, scapy-com into optional libraries
try:
    import gi
    gi.require_version('Gtk', '3.0')
    from gi.repository import Gtk as gtk
except ImportError:
    warn.append("gtk")
    apt_get_pkgs.append("python-gtk2")

try:
    import cairo
except ImportError:
    warn.append("cairo")
    apt_get_pkgs.append("python3-cairo")

try:
    import Crypto
except ImportError:
    err.append("crypto")
    apt_get_pkgs.append("python3-crypto")

try:
    import serial
except ImportError:
    err.append("serial")
    apt_get_pkgs.append("python3-serial")

# Ensure we have either pyUSB 0.x or pyUSB 1.x, but we now
#  prefer pyUSB 1.x moving forward. Support for 0.x may be deprecated.
try:
    import usb
except ImportError:
    err.append("usb")
    apt_get_pkgs.append("python3-usb")

try:
    import usb.core
except ImportError:
    warn.append("You are using pyUSB 0.x. Consider upgrading to pyUSB 1.x.")

# TODO: Ideally we would detect missing python3-dev and libgcrypt-dev to give better errors.

# Dot15d4 is a dep of some of the newer tools
try:
    from scapy.all import Dot15d4
except ImportError:
    warn.append("Scapy 802.15.4 (see README.md)")
    pip_pkgs.append("git+https://github.com/secdev/scapy.git#egg=scapy")

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
        print("\tpip install {}".format(' '.join(pip_pkgs)), file=sys.stderr)

if len(err) > 0:
    sys.exit(1)

zigbee_crypt = Extension('zigbee_crypt',
                         sources = ['zigbee_crypt/zigbee_crypt.c'],
                         libraries = ['gcrypt'],
                         include_dirs = ['/usr/local/include', '/usr/include', '/sw/include/', 'zigbee_crypt'],
                         library_dirs = ['/usr/local/lib', '/usr/lib','/sw/var/lib/']
                         )

setup(name        = 'killerbee',
      version     = '3.0.0',
      description = 'ZigBee and IEEE 802.15.4 Attack Framework and Tools',
      author = 'Joshua Wright, Ryan Speers',
      author_email = 'jwright@willhackforsushi.com, ryan@riverloopsecurity.com',
      license   = 'LICENSE.txt',
      packages  = ['killerbee', 'killerbee.openear', 'killerbee.zbwardrive'],
      scripts = ['tools/zbdump', 'tools/zbgoodfind', 'tools/zbid', 'tools/zbreplay',
                 'tools/zbconvert', 'tools/zbdsniff', 'tools/zbstumbler', 'tools/zbassocflood',
                 'tools/zbfind', 'tools/zbscapy', 'tools/zbwireshark', 'tools/zbkey',
                 'tools/zbwardrive', 'tools/zbopenear', 'tools/zbfakebeacon',
                 'tools/zborphannotify', 'tools/zbpanidconflictflood', 'tools/zbrealign', 'tools/zbcat',
                 'tools/zbjammer', 'tools/kbbootloader'],
      install_requires=['pyserial>=2.0', 'pyusb', 'pycrypto', 'rangeparser'],
                        #'git+https://github.com/secdev/scapy.git#egg=scapy'],
      # NOTE: pygtk doesn't install via distutils on non-Windows hosts
      ext_modules = [zigbee_crypt],
      )

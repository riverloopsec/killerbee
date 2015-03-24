# Introduction #

Some users may want to run KillerBee on BackTrack Linux distributions. We offer some specific notes here.

# Kali 6 #

KillerBee 1.0 comes installed by default. We strongly suggest updating to the trunk version or a newer release from this site.

When installing an updated version, note:
  * Download newer release and untar or SVN checkout to a location of your choice
  * Remove the old version and install required packages:
```
sudo rm -rf /usr/lib/pymodules/python2.7/killerbee
sudo apt-get install python-gtk2 python-cairo python-usb python-crypto python-serial python-dev libgcrypt-dev
```
  * cd into the new KillerBee download, and in the directory containing setup.py, run:
```
sudo python setup.py install
```
  * Test installation by running sudo zbid and other tools

# BackTrack 5.2 #

KillerBee 1.0 comes installed by default. We strongly suggest updating to the trunk version or a newer release from this site.

When installing an updated version, note:
  * Download newer release and untar or SVN checkout to a location of your choice
  * Remove the KillerBee 1.0 libraries:
```
rm -rf /usr/lib/python2.6/dist-packages/killerbee
```
  * cd into the new KillerBee download, and in the directory containing setup.py, run:
```
sudo python setup.py install
```
  * Test installation by running sudo zbid and other tools

# BackTrack 5.1 and earlier #

No special installation notes.
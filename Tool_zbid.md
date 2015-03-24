# Introduction #
tools/zbid is a simple script that uses the killerbee.kbutils to enumerate the attached radio interfaces.

# Usage #

```
sudo zbid
```

_Note: the command may take a while to run if numerous serial devices are attached._

Full usage:
```
$ zbid -h
usage: zbid [-h] [-i INCLUDE] [-g IGNORE]

Print a list of the attached KillerBee recognized devices to stdout. The -g
flag may be provided to ignore a serial device, such as an attached GPS serial
device which should be ignored by KillerBee.

optional arguments:
  -h, --help            show this help message and exit
  -i INCLUDE, --iface INCLUDE, --dev INCLUDE
  -g IGNORE, --gps IGNORE, --ignore IGNORE
```

# Output #

Example output for a computer with each type of currently supported device attached.

```
$ sudo zbid
Dev	Product String	Serial Number
005:006	KILLERB001	A50400A01C25
/dev/ttyUSB1	Dartmouth Freakduino	
/dev/ttyUSB0	GoodFET TelosB/Tmote	
Found 3 devices.
```

If you want to look for network-connected sniffers, the -i of the IP address is needed:

```
$ zbid -i 10.10.10.2
           Dev Product String       Serial Number
  /dev/ttyUSB0 GoodFET TelosB/Tmote           
    10.10.10.2 Wislab Sniffer v0.5  00:1a:b6:00:0b:ff
```
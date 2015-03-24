# Introduction #
tools/zbstumbler is a simple script that transmits beacon request frames to the broadcast address while
channel hopping to identify ZC/ZR (Coordinator and Router) devices. It depends on these devices responding to the beacon request frame while it's on the channel. Different interfaces for KillerBee may turn around

# Usage #
zbstumbler [-i devnumstring] [-s per-channel delay] [-c channel] [-w report.csv]

```
$ sudo zbstumbler
```
Or define an interface. Sudo is only needed if root permissions are needed to access the interface.
```
$ zbstumbler -i /dev/ttyUSB0
```

_Note: it's best to provide an interface using -i, if you know which one you want to use._

# Output #

Example output for a computer with each type of currently supported device attached.

```
TODO
```
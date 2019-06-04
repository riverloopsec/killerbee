# Frequently Asked Questions

We often receive the same questions via email, and include this to answer some of the most common ones.

## Installation

#### Failed install due to Python.h missing

- Appears as:
    ```bash
    ...
    zigbee_crypt/zigbee_crypt.c:13:10: fatal error: Python.h: No such file or directory
     #include <Python.h>
              ^~~~~~~~~~
    compilation terminated.
    error: command 'x86_64-linux-gnu-gcc' failed with exit status 1
    ```

- Cause:
The requirements were not installed per `README.md`, specifically the Python development package.

- Fix:
Install the requirement, such as `sudo apt-get install -y python-dev` (or `python3-dev`).

#### Failed install due to gcrypt.h missing

- Appears as:
    ```bash
    ...
    zigbee_crypt/zigbee_crypt.c:15:10: fatal error: gcrypt.h: No such file or directory
     #include <gcrypt.h>
              ^~~~~~~~~~
    compilation terminated.
    error: command 'x86_64-linux-gnu-gcc' failed with exit status 1
    ```

- Cause:
The requirements were not installed per `README.md`, specifically the gcrypt development package.

- Fix:
Install the requirement, such as `sudo apt-get install -y libgcrypt-dev`.

## Device Usage

### Atmel RZUSBSTICK

#### Flashing

See `firmware/README.md` for details.

#### ValueError device has no langid

- Appears as:
    ```bash
    zbid
    ...
    Traceback (most recent call last):
      File "/usr/local/bin/zbid", line 23, in <module>
        show_dev(gps=arg_gpsdev, include=args.include)
      File "/usr/local/lib/python2.7/dist-packages/killerbee/__init__.py", line 46, in show_dev
        for dev in kbutils.devlist(vendor=vendor, product=product, gps=gps, include=include):
      File "/usr/local/lib/python2.7/dist-packages/killerbee/kbutils.py", line 285, in devlist
        devlist = devlist_usb_v1x(vendor, product)
      File "/usr/local/lib/python2.7/dist-packages/killerbee/kbutils.py", line 215, in devlist_usb_v1x
        usb.util.get_string(dev, dev.iProduct),     \
      File "/usr/lib/python2.7/dist-packages/usb/util.py", line 314, in get_string
        raise ValueError("The device has no langid")
    ValueError: The device has no langid
    ```

- Cause: USB permissions

- Fix: Run as sudo, or change the permissions to the USB device so your user can query it

### Apimote v4beta

#### Does not enumerate reliably

- Appears as:
  - Device doesn't show up in `zbid` sometimes
  - "Serial timeout" message printed to console during running commands

- Cause:
As detailed on the page for this device, it is in beta due to instability observed with it establishing
a serial sync with some hosts.

- Fix:
  - Help improve it, likely by working on the settings for the FTDI chip on the PCB
  - Specify the device using `-i` when you run commands, so enumeration doesn't need to be run each time
  - Unplug and replug the device as needed

#### Does not get frames received

- Appears as: Missing frames that you expect and see with other devices
- Cause: Often we find that users are not attaching the antenna as required.
- Fix: As detailed in the product documentation, you must either:
  - have an appropriate antenna attached to the RP-SMA port
  - or, move the component C501 on the PCB to select the internal antenna

#### Shows v2 when it enumerates

- Appears as: `zbid` lists the device as 'v2'
- Cause: expected behavior, as from the software side only v1 is different than v2-v4, and thus it doesn't see a difference
- Fix: N/A

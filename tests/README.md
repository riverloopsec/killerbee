# Testing 

## Setup

```
$ pip3 install nose2
$ cd tests/
$ APIMOTE_DEVSTRING=/dev/ttyUSB0 nose2 -v
```

## Test Coverage 

Tests have currently only been written for and run using an Apimote, for many tests interaction with the device is necessary and the environment variable `APIMOTE_DEVSTRING` should point to the device.

A few functions for scapy_extension require reading packet or extracting information from captures, these fixtures are currently not in place to test these functions.

### KilleBbee Core

`killerbee/__init__.py`

| funciton | test | notes |
| -------- | ---- | ----- |
| getKillerBee | :white_check_mark: | |
| kb_dev_list | :x: | Deprecated |
| show_dev | :white_check_mark: | |
| KillerBee.__init__ | :white_check_mark: | |
| KillerBee.close | :white_check_mark: | |
| KillerBee.get_dev_info | :white_check_mark: | |
| KillerBee.check_capability | :white_check_mark: | |
| KillerBee.is_valid_channel | :white_check_mark: | |
| KillerBee.frequency | :white_check_mark: | |
| KillerBee.get_capabilities | :white_check_mark: | |
| KillerBee.enter_bootloader | :white_check_mark: | Apimote driver does not support function |
| KillerBee.get_bootloader_version | :white_check_mark: | Apimote driver does not support function |
| KillerBee.get_bootloader_signature | :white_check_mark: | Apimote driver does not support function | 
| KillerBee.bootloader_sign_on | :white_check_mark: | Apimote driver does not support function |
| KillerBee.bootloader_start_application | :white_check_mark: | Apimote driver does not support function |
| KillerBee.channel (property) | :white_check_mark: | |
| KillerBee.page (property) | :white_check_mark: | |
| KillerBee.set_channel | :white_check_mark: | |
| KillerBee.inject | :white_check_mark: | |
| KillerBee.pnext | :white_check_mark: | |
| KillerBee.jammer_on | :white_check_mark: | Feature not supported on Apimote |
| KillerBee.jammer_off | :white_check_mark: | Feature not supported on Apimote |

### Apimote Driver 

`killerbee/dev_apimote.py`

| funciton | test | notes |
| -------- | ---- | ----- |
| __init__ | :white_check_mark: | |
| close | :white_check_mark: | |
| get_capabilities | :white_check_mark: | |
| get_dev_info | :white_check_mark: | |
| check_capabilities | :white_check_mark: | |
| sniffer_on | :white_check_mark: | |
| sniffer_off | :white_check_mark: | |
| set_channel | :white_check_mark: | |
| inject | :white_check_mark: | |
| pnext | :white_check_mark: | |
| ping | :white_check_mark: | Feature not supported on Apimote |
| jammer_on | :white_check_mark: | Feature not supported on Apimote |
| jammer_off | :white_check_mark: | Feature not supported on Apimote |
| set_sync | :white_check_mark: | |

### KillerBee Utils
`killerbee/kbutils.py`

| funciton | test | notes |
| -------- | ---- | ----- |
| KBCapabilities.__init__ | :white_check_mark: | |
| KBCapabilities.check | :white_check_mark: | |
| KBCapabilities.getlist | :white_check_mark: | |
| KBCapabilities.setcapab | :white_check_mark: | |
| KBCapabilities.require | :white_check_mark: | |
| KBCapabilities.is_valid_channel | :white_check_mark: | |
| KBCapabilities.frequency | :white_check_mark: | |
| findFromList.__init__ | :white_check_mark: | |
| findFromList.call | :x: | |
| findFromListAndBusDevId.__init__ | :white_check_mark: | |
| findFromListAndBusDevId.call | :x: | |
| devlist_usb_v1x | :x: | |
| devlist_usb_v0x| :x: | USB v0x Deprecated |
| devlist | :white_check_mark: | |
| get_serial_devs | :x: | Deprecated |
| getSerialDeviceString | :white_check_mark: | |
| get_serial_ports | :white_check_mark: | |
| isIpAddr | :white_check_mark: | |
| isgoodfetccspi | :white_check_mark: | Tested only on Apimote |
| iszigduino | :white_check_mark: | Tested only on Apimote |
| issl_nodetest | :white_check_mark: | Tested only on Apimote |
| issl_beehive | :white_check_mark: | Tested only on Apimote |
| isfreakduino | :white_check_mark: | Tested only on Apimote |
| search_usb | :white_check_mark: | |
| search_usb_bus_v0x | :x: | USB v0x Deprecated |
| hexdump | :x: | |
| randbytes | :white_check_mark: | |
| randmac | :white_check_mark: | |
| makeFS | :white_check_mark: | |
| bytearray_to_bytes | :white_check_mark: | |


### ZigbeeCrypt 
`zigbee_crypt/zigbeecrypt.c`

| funciton | test | notes |
| -------- | ---- | ----- |
| decrypt_ccm | :white_check_mark: | |
| encrypt_ccm | :white_check_mark: | |
| sec_key_hash | :white_check_mark: | |

### KBScapyExt
`killerbee/scapy_extensions.py`

| funciton | test | notes |
| -------- | ---- | ----- |
| kbdev | :white_check_mark: | |
| kbsendp | :white_check_mark: | |
| kbsrp | :x: | |
| kbsrp1 | :x: | |
| kbsniff | :x: | |
| kbrdpcap | :white_check_mark: | *tracemalloc warning |
| kdwrpcap | :white_check_mark: | *tracemalloc warning |
| kbrddain | :x: | Deprecated |
| kbwrdain | :x: | Deprecated |
| kbkeysearch | :x: | |
| kbgetnetworkkey | :x: | |
| kbtshark | :x: | |
| kbrandmac | :white_check_mark: | |
| kbdecrypt | :white_check_mark: | |
| kbencrypt | :white_check_mark: | |
| kbgetmiclen | :white_check_mark: | |
| kbgetpanid | :x: | |


# Typing

## Setup

```
$ pip3 install mypy
$ mypy killerbee
```

## Type Coverage 

### KilleBbee Core

`killerbee/__init__.py`

| funciton | types | notes |
| -------- | ---- | ----- |
| getKillerBee | :white_check_mark: | |
| show_dev | :white_check_mark: | |
| KillerBee.__init__ | :white_check_mark: | |
| KillerBee.close | :white_check_mark: | |
| KillerBee.get_dev_info | :white_check_mark: | |
| KillerBee.check_capability | :white_check_mark: | |
| KillerBee.is_valid_channel | :white_check_mark: | |
| KillerBee.frequency | :white_check_mark: | |
| KillerBee.get_capabilities | :white_check_mark: | |
| KillerBee.enter_bootloader | :white_check_mark: | |
| KillerBee.get_bootloader_version | :white_check_mark: | |
| KillerBee.get_bootloader_signature | :white_check_mark: | | 
| KillerBee.bootloader_sign_on | :white_check_mark: | |
| KillerBee.bootloader_start_application | :white_check_mark: | |
| KillerBee.channel (property) | :white_check_mark: | |
| KillerBee.page (property) | :white_check_mark: | |
| KillerBee.set_channel | :white_check_mark: | |
| KillerBee.inject | :white_check_mark: | |
| KillerBee.pnext | :white_check_mark: | |
| KillerBee.jammer_on | :white_check_mark: | |
| KillerBee.jammer_off | :white_check_mark: | |

### Apimote Driver 

`killerbee/dev_apimote.py`

| funciton | types | notes |
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
| ping | :white_check_mark: | |
| jammer_on | :white_check_mark: | |
| jammer_off | :white_check_mark: | |
| set_sync | :white_check_mark: | |

### KillerBee Utils
`killerbee/kbutils.py`

| funciton | types | notes |
| -------- | ---- | ----- |
| KBCapabilities.__init__ | :white_check_mark: | |
| KBCapabilities.check | :white_check_mark: | |
| KBCapabilities.getlist | :white_check_mark: | |
| KBCapabilities.setcapab | :white_check_mark: | |
| KBCapabilities.require | :white_check_mark: | |
| KBCapabilities.is_valid_channel | :white_check_mark: | |
| KBCapabilities.frequency | :white_check_mark: | |
| findFromList.__init__ | :white_check_mark: | |
| findFromList.call | :white_check_mark: | |
| findFromListAndBusDevId.__init__ | :white_check_mark: | |
| findFromListAndBusDevId.call | :white_check_mark: | |
| devlist_usb_v1x | :white_check_mark: | |
| devlist | :white_check_mark: | |
| get_serial_devs | :white_check_mark: | Deprecated |
| getSerialDeviceString | :white_check_mark: | |
| get_serial_ports | :white_check_mark: | |
| isIpAddr | :white_check_mark: | |
| isgoodfetccspi | :white_check_mark: | |
| iszigduino | :white_check_mark: | |
| issl_nodetest | :white_check_mark: | |
| issl_beehive | :white_check_mark: | |
| isfreakduino | :white_check_mark: | |
| search_usb | :white_check_mark: | |
| hexdump | :x: | |
| randbytes | :white_check_mark: | |
| randmac | :white_check_mark: | |
| makeFCS | :white_check_mark: | |
| bytearray_to_bytes | :white_check_mark: | |

### KBScapyExt
`killerbee/scapy_extensions.py`

| funciton | types| notes |
| -------- | ---- | ----- |
| kbdev | :white_check_mark: | |
| kbsendp | :white_check_mark: | |
| kbsrp | :white_check_mark: | |
| kbsrp1 | :white_check_mark: | |
| kbsniff | :white_check_mark: | |
| kbrdpcap | :white_check_mark: | |
| kdwrpcap | :white_check_mark: | |
| kbrddain | :white_check_mark: | |
| kbwrdain | :white_check_mark: | |
| kbkeysearch | :white_check_mark: | |
| kbgetnetworkkey | :white_check_mark:: | |
| kbtshark | :white_check_mark: | |
| kbrandmac | :white_check_mark: | |
| kbdecrypt | :white_check_mark: | |
| kbencrypt | :white_check_mark: | |
| kbgetmiclen | :white_check_mark: | |
| kbgetpanid | :white_check_mark: | |

### DainTree
`killerbee/daintree.py`

| funciton | types| notes |
| -------- | ---- | ----- |
| DaintreeDumper.__init__ | :white_check_mark: | |
| DaintreeDumper.pcap_dump | :white_check_mark: | |
| DaintreeDumper.pwrite | :white_check_mark: | |
| DaintreeDumper.close | :white_check_mark: | |
| DaintreeReader.__init__ | :white_check_mark: | |
| DaintreeReader.close | :white_check_mark: | |
| DaintreeReader.pnext | :white_check_mark: | |

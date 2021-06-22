# Architecture

## Overview

The below table is intended to help orient users to the different parts of the KillerBee codebase,
and share overall thoughts for planned future changes to allow for comments, feedback, and planning.

| Category | Sub-category | Description | Examples | Notes | 
| -------- | ------------ | ----------- | -------- | ----- | 
| core | | The library that is used to interact with the radio peripheral | `pnext()`, `inject()`, `set_channel()`, `sniffer_on()/off()` | |
| | drivers | Device specific implementation of "core" functionality | `dev_*.py` | Will be moved to a sub-directory. |
| utilities | | Convenience functions for working with ZigBee | | |
| | pcap | | | Plan to migrate to using scapy fully for this |
| | device | | `devlist()`, `is_\[device\]()` | Possibly move this into core |
| | zigbee | | `makeFCS()`, `randmac()`, `extractkey()` | |
| | crypto | | `encrypt_ccm()`, `decrypt_ccm()`, `hash_mmo()` | |
| | rf4ce | | `derivekey()` | Planned future features. |
| | home-automation | | `disarm()`, `sensor_trigger()`, `tamper()` | Planned future features. |
| scapy-extension | | A wrapper around "core" and "utility" for convenience when working alongside scapy or as-if working with scapy | `kbsend()`, `kbsniff()` | |
| | home-automation layer | | | Planned future features. |
| tools | simple | Simple script wrappers around "core" or "utility" functionality | e.g., `zbcat`, `zbdump`, `zbwireshark`, `zbid`, `zbcrypto` | |
| | fully-featured | Other, more complex tools | `zbwardrive`, `openear` | These will be promoted to their own repository |

## Additional Goals 

* Migrate remaining tools and utilities to use Dot15d4 Scapy for packet construction
* Move device configuration to runtime options rather than "compile" time
* Add unittesting coverage (see `tests/` for coverage so far)
* Add pytypes
* Move documentation to modern framework such as Sphinx
* Publish documentation to [readthedocs.io](https://readthedocs.io)
* Publish KillerBee to pypi to simplify installation
* Have KillerBee updated in distros (e.g., Kali)

Firmware
================

RZUSBSTICK
==================

This needs to be flashed using external tools. Compiled firmware is:
- kb-rzusbstick-001.hex: Initial version
- kb-rzusbstick-002.hex: Newer version with EEMAGIC=0xFF, reported to be needed on newer models

The source is included in src/kb-rzusbstick/ folder. It should build on Windows and Ubuntu.

ApiMote
==================

The firmware for this is mainained and built using the GoodFET repository.

Compiled firmware versions are stored here for ease of use. However, you should often not need to use these as ApiMote v4betas provided by River Loop Security are all pre-flashed with the latest version.

To flash it, plug in the ApiMote v3 or v4beta and then run the flash\_apimote.sh script.
Note that it may sometimes take two attempts to flash correctly if the first time doesn't syncronize and times out. 



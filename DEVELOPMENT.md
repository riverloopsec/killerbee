KillerBee Developer Guidance
================

This document seeks to set standards for development.
We welcome and greatly appreciate improvements and contributions!

Python Versions
================

Please write all code to be compatible with Python 2.7+ and 3.5+.

Branching
================

All contributions should be targeted at the `develop` branch.

The `master` branch shall contain the currently released version at all times.

To cut a release, we will branch `develop` to a `release/major.minor` branch, e.g., `release/2.1`.
That branch will serve as the release candidate.
A PR to the `master` branch from the `release/*` branch will promote that code to the most recent release.

Tools
================

Scripts in the `tools/` folder should be designed for common end-user use.
More tailored helper scripts or other things you wish to keep but aren't ready for "primetime" should go in the
 `scripts/` folder.
All tools must accept standard command line flags and be parsed by `argparse`. See an existing tool like `zbdump` for
 examples of expected flags.

Tools should be prefixed by `zb` in the name if they operate on IEEE 802.15.4 or ZigBee RF/PCAP/etc.
They may be prefixed instead by `kb` if they are tools to setup/configure KillerBee itself.

Contributions
================

KillerBee is distributed under a BSD license, see LICENSE for details.

Contributors who submit pull requests, suggestions, or otherwise agree that the project maintainers
 are free to use, modify, re-license, sublicense, etc. them without limitation.
They also warrant that they have the right to contribute code in this way, and that nothing in them is
 patented nor prohibited from disclosure under an NDA.

Questions
==============
Please use the ticketing system at https://github.com/riverloopsec/killerbee/issues.

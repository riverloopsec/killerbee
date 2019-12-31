# Make core client functions available without prefix.
#
# This file is Copyright (c) 2010 by the GPSD project
# BSD terms apply: see the file COPYING in the distribution root for details.

# The 'client' module exposes some C utility functions for Python clients.
# The 'packet' module exposes the packet getter via a Python interface.
__all__ = ["gps", "client"]
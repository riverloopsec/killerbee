README: zbwardrive 

zbWarDrive is a tool that seeks to achieve optimal coverage of networks
with using only the available capture interfaces. It discovers
available interfaces and uses one to inject beacon requests
and listen for respones across channels. Once a network is found
on a channel, it assigns another device to continuously capture
traffic on that channel to a PCAP file.

Running it with the -d option attempts to log data to a database using
the KillerBee DBLogger (killerbee/dblog.py). This takes connection 
information for a pre-configured MySQL database from killerbee/config.py.
zbWarDrive will always try to write PCAP files (one per channel) locally
as well, and if -d is not defined, the PCAPS are the only output written.

GPS data logging support to be added.
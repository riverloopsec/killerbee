#!/usr/bin/env python

from killerbee.zbwardrive import *

# Command line main function
if __name__=='__main__':
    # Command line parsing
    parser = argparse.ArgumentParser(description="""
Use any attached KillerBee-supported capture devices to preform a wardrive,
by using a single device to iterate through channels and send beacon requests
while other devices are assigned to capture all packets on a channel after
it is selected as 'of interest' which can change based on the -a flag.
""")
    parser.add_argument('-v', '--verbose', dest='verbose', action='store_true',
                        help='Produce more output, for debugging')
    parser.add_argument('-d', '--db', dest='dblog', action='store_true',
                        help='Enable KillerBee\'s log-to-database functionality')
    parser.add_argument('-a', '--agressive', dest='agressive', action='store_true',
                        help='Initiate capture on channels where packets were seen, even if no beacon response was received')
    parser.add_argument('-g', '--gps', dest='gps', action='store_true',
                        help='Connect to gpsd and grab location data as available to enhance PCAPs')
    args = parser.parse_args()

    # try-except block to catch keyboard interrupt.
    zbdb = None
    gpsp = None
    try:
        # Some shared state for multiprocessing use
        manager = Manager()
        devices = manager.dict()
        currentGPS = None
        if args.gps:
            currentGPS = manager.dict()
            gpsp = Process(target=gpsdPoller, args=(currentGPS, ))
            gpsp.start()

        zbdb = ZBScanDB()
        #TODO check return value from startScan
        startScan(zbdb, currentGPS, verbose=args.verbose, dblog=args.dblog, agressive=args.agressive)
        zbdb.close()

    except KeyboardInterrupt:
        print 'Shutting down'
        if zbdb != None: zbdb.close()
        if gpsp != None: gpsp.terminate()

#!/usr/bin/env python

# ZBWarDrive
# rmspeers 2010-13
# ZigBee/802.15.4 WarDriving Platform

from time import sleep
from usb import USBError

from killerbee import KillerBee, kbutils
from .db import ZBScanDB
from .scanning import doScan

# GPS Poller
def gpsdPoller(currentGPS):
    '''
    @type currentGPS multiprocessing.Manager dict manager
    @arg currentGPS store relavent pieces of up-to-date GPS info
    '''
    from . import gps
    gpsd = gps.gps()
    gpsd.poll()
    gpsd.stream()

    try:
        while True:
            gpsd.poll()
            if gpsd.fix.mode > 1: #1=NO_FIX, 2=FIX, 3=DGPS_FIX
                lat = gpsd.fix.latitude
                lng = gpsd.fix.longitude
                alt = gpsd.fix.altitude
                #TODO do we want to use the GPS time in any way?
                currentGPS['lat'] = lat
                currentGPS['lng'] = lng
                currentGPS['alt'] = alt
            else:
                print("Waiting for a GPS fix.")
                #TODO timeout lat/lng/alt values if too old...?
    except KeyboardInterrupt:
        print("Got KeyboardInterrupt in gpsdPoller, returning.")
        return

# startScan
# Detects attached interfaces
# Initiates scanning using doScan()
def startScan(zbdb, currentGPS, verbose=False, dblog=False, agressive=False, include=[], ignore=None):
    try:
        kb = KillerBee()
    except USBError as e:
        if e.args[0].find('Operation not permitted') >= 0:
            print('Error: Permissions error, try running using sudo.')
        else:
            print('Error: USBError:', e)
        return False
    except Exception as e:
        print('Error: Issue starting KillerBee instance:', e)
        return False
    for kbdev in kbutils.devlist(gps=ignore, include=include):
        print('Found device at %s: \'%s\'' % (kbdev[0], kbdev[1]))
        zbdb.store_devices(
            kbdev[0], #devid
            kbdev[1], #devstr
            kbdev[2]) #devserial
    kb.close()
    doScan(zbdb, currentGPS, verbose=verbose, dblog=dblog, agressive=agressive)
    return True




#!/usr/bin/env python
# ZBScanner
# rmelgares 2011
# Promiscious capture on multiple channels at once

import gps, time, os, signal, sys, operator, threading
import string, socket, struct, datetime

from killerbee import *
import Queue

# Globals
session = ""
active_queues = []
arg_verbose = False
arg_ppi = False
arg_db = False
arg_gps = False
arg_gps_devstring = ""
latitude = ""
longitude = ""
altitude = ""
last_seen = ""

def broadcast_event(data):
    ''' Send broadcast data to all active threads '''
    print "\nShutting down threads." 
    for q in active_queues:
        q.put(data)

class LocationThread(threading.Thread):
    ''' Thread to update gps location from gpsd '''
    def __init__(self):
        global active_queues
        threading.Thread.__init__(self)
        self.mesg = Queue.Queue()
        active_queues.append(self.mesg)

    def run(self):
        global session
        global active_queues
        global longitude, latitude, altitude, last_seen
        message = ""
        last_seen = datetime.datetime.now()
        while(1):
            try:
                message = self.mesg.get(timeout=.00001)
            except Queue.Empty:
                pass
            if message == "shutdown":
                break
            session.poll()
            t_latitude = session.fix.latitude
            t_longitude = session.fix.longitude
            t_altitude = session.fix.altitude
            if(t_latitude != latitude and t_longitude != longitude and t_altitude != altitude):
                latitude  = t_latitude
                longitude = t_longitude
                altitude  = t_altitude
                last_seen = datetime.datetime.now()
            else:
                end_time = datetime.datetime.now()
                elapsed_time = end_time - last_seen
                print chr(0x1b) + "[2;5fElapsed time since last location change: %s" % str(elapsed_time) 
            print chr(0x1b) + "[3;5fLat: %f, Long: %f, Alt: %f." % (latitude, longitude, altitude)
            time.sleep(1)
 

class CaptureThread(threading.Thread):
    ''' Thread to capture on a given channel, using a given device, to a given pcap file, exits when it receives a broadcast shutdown message via Queue.Queue'''
    def __init__(self, dev, channel, pd):
        global active_queues
        threading.Thread.__init__(self)
        self.mesg = Queue.Queue()
        active_queues.append(self.mesg)
        self.channel = channel
        self.freq = (channel - 10) * 5 + 2400
        self.dev = dev
        self.pd = pd
        self.packetcount = 0

    def run(self):
        global active_queues
        global longitude, latitude, altitude
        self.kb = KillerBee(device=self.dev)
        self.kb.set_channel(self.channel)
        self.kb.sniffer_on()
        
        # loop capturing packets to dblog and file
        message = ""
        while (True):
            try:
                message = self.mesg.get(timeout=.00001)
            except Queue.Empty:
                pass
            if message == "shutdown":
                break
            packet = self.kb.pnext()
            if packet != None:
                self.packetcount+=1
                if arg_gps:
                    gpsdata = (longitude, latitude, altitude)
                    self.pd.pcap_dump(packet['bytes'], ant_dbm=packet['dbm'], freq_mhz=self.freq, location=map(float, gpsdata))
                    if arg_db:self.kb.dblog.add_packet(full=packet, location=gpsdata)
                else:
                    self.pd.pcap_dump(packet['bytes'])
                    if arg_db:self.kb.dblog.add_packet(full=packet)
                print chr(0x1b) + "[%d;5fChannel %d: %d packets captured." % (self.channel - 6, self.channel, self.packetcount)
        # trigger threading.Event set to false, so shutdown thread
        if arg_verbose:
            print "%s on channel %d shutting down..." % (threading.currentThread().getName(), self.channel)
        self.kb.sniffer_off()
        self.kb.close()
        self.pd.close()
        print "%d packets captured on channel %d" % (self.packetcount, self.channel)

def signal_handler(signal, frame):
    ''' Signal handler called on keyboard interrupt to exit threads and exit scanner script'''
    os.system('clear') 
    broadcast_event("shutdown")
    time.sleep(1)
    sys.exit(0)

def main(args):
    global arg_verbose
    global arg_gps, arg_gps_devstring
    global session
    global longitude, latitude, altitude
    # parse command line options
    while len(args) > 1:
        op = args.pop(1)
        if op == '-v':                          # Verbose mode
            arg_verbose = True
        if op == '-g':                          # GPS device
            arg_gps_devstring = sys.argv.pop(1)
            arg_gps = True
        if op == '-p':                          # PCAP PPI geolocation support
            arg_ppi = True
        if op == '-d':                          # Database logging
            arg_db = True

    signal.signal(signal.SIGINT, signal_handler)

    if arg_gps:
        kbdev_info = kbutils.devlist(gps=arg_gps_devstring)
    else:
        kbdev_info = kbutils.devlist()
    channel = 11
    print "Found %d devices." % len(kbdev_info)
    if len(kbdev_info) < 1:
        exit(1)
    if arg_gps == True:
        print "Initializing GPS device %s ... "% (arg_gps_devstring),
        session = gps.gps()
        session.poll()
        session.stream()
        print "Waiting for fix... ",
        while(session.fix.mode == 1):
            session.poll()
        print "Fix acquired!"
        t = LocationThread()
        t.start()
    time.sleep(2)
    for i in range(0, len(kbdev_info)):
            if kbdev_info[i][0] == arg_gps_devstring:
                print "Skipping device %s" % arg_gps_devstring
            else:
                print 'Device at %s: \'%s\'' % (kbdev_info[i][0], kbdev_info[i][1])
                if channel <= 26:
                    print '\tAssigning to channel %d.' % channel
                timeLabel = datetime.datetime.now().strftime('%Y%m%d-%H%M')
                fname = 'zb_c%s_%s.pcap' % (channel, timeLabel) #fname is -w equiv
                pcap = PcapDumper(DLT_IEEE802_15_4, fname, ppi=arg_ppi)
                t = CaptureThread(kbdev_info[i][0], channel, pcap)
                t.start()
                channel += 1
    os.system('clear')
    print chr(0x1b) + "[1;5fLive Stats:"
    while True: pass

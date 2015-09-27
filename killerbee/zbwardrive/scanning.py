#!/usr/bin/env python

import datetime
import logging
import multiprocessing
import Queue
import signal
import socket
import string
import struct
import sys
import time
import traceback

from killerbee import *
try:
    from scapy.all import Dot15d4, Dot15d4Beacon
except ImportError:
    log_message = 'This Requires Scapy (Dot15d4) To Be Installed.'
    print log_message
    logging.error(log_message)
    from sys import exit
    exit(-1)

# TODO: We're assuming that the device can inject
# ug... so many parameters
class Scanner(multiprocessing.Process):
    def __init__(self, device, devstring, channel, channels,
                 verbose, currentGPS, kill, output,
                 scanning_time, capture_time):
        multiprocessing.Process.__init__(self)
        self.dev = device             # KB device
        self.devstring = devstring    # Name of the device (for logging) 
        self.channels = channels      # Shared queue of channels
        self.channel = channel        # Shared memory of current channel
        self.verbose = verbose        # Verbose flag
        self.currentGPS = currentGPS  # Shared memorf of GPS data
        self.kill = kill              # Kill event
        self.output = output          # Output folder
        self.scanning_time = scanning_time  # How long to wait on a channel to see if it's active
        self.capture_time = capture_time    # How long to record on an active channel

    def run(self):
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        log_message = "Scanning with {}".format(self.devstring)
        if self.verbose:
            print log_message
        logging.debug(log_message)

        beacon = "\x03\x08\x00\xff\xff\xff\xff\x07" # beacon frame
        beaconp1 = beacon[0:2]  # beacon part before seqnum field
        beaconp2 = beacon[3:]   # beacon part after seqnum field
        # TODO: Do we want to keep sequence numbers unique across devices?
        seqnum = 0              # seqnum to use (will cycle)

        while(1):
            if self.kill.is_set():
                log_message = "{}: Kill event caught".format(self.devstring)
                if self.verbose:
                    print log_message
                logging.debug(log_message)
                return

            # Try to get the next channel, if there aren't any, sleep and try again
            # It shouldn't be empty unless there are more devices than channels
            try:
                self.channel.value = self.channels.get(False)
            except Queue.Empty:
                time.sleep(1)
                continue

            # Change channel
            try:
                self.dev.set_channel(self.channel.value)
            except Exception as e:
                log_message = "%s: Failed to set channel to %d (%s)." % (
                    self.devstring, self.channel.value, e)
                if self.verbose: 
                    print log_message
                logging.error(log_message)
                return

            # Craft and send beacon
            if seqnum > 255:
                seqnum = 0
            beaconinj = beaconp1 + "%c" % seqnum + beaconp2
            seqnum += 1
            log_message = "{}: Injecting a beacon request on channel {}".format(
                self.devstring, self.channel.value) 
            if self.verbose:
                print log_message
            logging.debug(log_message)
            try:
                self.dev.inject(beaconinj)
            except Exception, e:
                log_message = "%s: Unable to inject packet (%s)." % (
                    self.devstring, e)
                if self.verbose:
                    print log_message
                logging.error(log_message)
                return

            # Listen for packets
            endtime = time.time() + self.scanning_time
            try:
                while (endtime > time.time()):
                    # Get any packets (blocks for 100 usec)
                    packet = self.dev.pnext()
                    if packet != None:
                        log_message = "{}: Found a frame on channel {}".format(
                            self.devstring, self.channel.value)
                        if self.verbose:
                            print log_message
                        logging.debug(log_message)
                        pdump = self.create_pcapdump()
                        self.dump_packet(pdump, packet)
                        self.capture(pdump)
                        break
            except Exception as e:
                log_message = "%s: Error in capturing packets (%s)." % (
                    self.devstring, e)
                if self.verbose:
                    print log_message
                    print traceback.format_exc()
                logging.error(log_message)
                logging.error(traceback.format_exc())
                return

            self.dev.sniffer_off()

            # Add channel back to the queue
            self.channels.put(self.channel.value)

    # Captures packets
    def capture(self, pdump):
        packet_count = 1 # We start already having captured one

        # The sniffer should already be on
        log_message = "{}: capturing on channel {}".format(self.devstring, self.channel.value)
        if self.verbose:
            print log_message
        logging.debug(log_message)

        # Loop and capture packets
        endtime = time.time() + self.capture_time
        while(endtime > time.time()):
            packet = self.dev.pnext()
            if packet != None:
                packet_count += 1
                self.dump_packet(pdump, packet)

        # All done
        pdump.close()
        log_message =  "{}: {} packets captured on channel {}".format(
            self.devstring, packet_count, self.channel.value)
        if self.verbose:
            print log_message
        logging.debug(log_message)

        
    def create_pcapdump(self):
        # Prep the pcap file
        time_label = datetime.datetime.utcnow().strftime('%Y%m%d-%H%M%S') 
        fname = '/zb_c%s_%s.pcap' % (self.channel.value, time_label) #fname is -w equiv
        return PcapDumper(DLT_IEEE802_15_4, fname, ppi=True, folder=self.output)

    
    def dump_packet(self, pdump, packet):
        rf_freq_mhz = (self.channel.value - 10) * 5 + 2400
        try:
            # Do the GPS if we can
            # Use KillerBee's hack to check the lat to see if GPS data is there
            if self.currentGPS != None and 'lat' in self.currentGPS:
                pdump.pcap_dump(packet[0], freq_mhz=rf_freq_mhz, ant_dbm=packet['dbm'],
                             location=(self.currentGPS['lng'], self.currentGPS['lat'], 
                                       self.currentGPS['alt']))
            else:
                print "GSP: {} {}".format((self.currentGPS != None), ('lat' in self.currentGPS))
                pdump.pcap_dump(packet[0], freq_mhz=rf_freq_mhz, ant_dbm=packet['dbm'])
        except IOError as e:
            log_message = "%s: Unable to write pcap (%s)." % (
                self.devstring, e)
            raise
        

# http://stackoverflow.com/questions/492519/timeout-on-a-python-function-call
class TimeoutError(Exception):
    pass

def timeoutHandler(signum, frame):
    raise TimeoutError()

    
# Takes a device id and returns the Zigbee device
# We make this its own function so we can time it
# and reset if it takes too long
# (The api-motes have a habit of timing out at start)
def create_device(device_id, verbose=False, timeout=10, tries_limit=5):
    old_handler = signal.signal(signal.SIGALRM, timeoutHandler)
    tries = 0
    while(1):
        signal.alarm(10)
        try:
            kbdevice = KillerBee(device=device_id)
            break
        except TimeoutError:
            log_message = "{}: Creation timeout (try={}/{})".format(
                device_id, tries, tries_limit)
            if verbose:
                print log_message
            logging.warning(log_message)
            tries += 1
            if tries >= tries_limit:
                log_message = "(%s): Failed to sync" % (device_id)
                if verbose:
                    print log_message
                logging.warning(log_message)
                raise Exception(log_message)
        finally:
            signal.alarm(0)
    signal.signal(signal.SIGALRM, old_handler)
    return kbdevice


def doScan(devices, currentGPS, verbose=False,  
           output='.', scanning_time=2, capture_time=5):
    timeout = 10    # How long to wait for each zigbee device to sync
    tries_limit = 5 # How many retries to give a zigbee device to sync
    scanners = []   # Stored information about each Scanner class we spawn
    channels = multiprocessing.Queue() # Keeps track of channels

    # Add the channels to the queue
    for i in range(11,26):
        channels.put(i)

    # Sync the devices and init the Scanners
    for device in devices:
        log_message =  "Creating {}".format(device[0])
        if verbose:
            print log_message
        logging.debug(log_message)

        # Create Scanner
        kill_event = multiprocessing.Event()
        channel = multiprocessing.Value('i',0)
        kbdevice = create_device(
            device[0], verbose=verbose, timeout=timeout,
            tries_limit=tries_limit)
        scanner_proc = Scanner(
            kbdevice, device[0], channel, channels,  verbose,
            currentGPS, kill_event, output, 
            scanning_time, capture_time)

        # Add scanner information to scanners list
        s = {}
        s["dev"] = kbdevice
        s["devstring"] = device[0]
        s["channel"] = channel
        s["proc"] = scanner_proc
        s["kill"] = kill_event
        scanners.append(s)

    # Start up the Scanners
    for s in scanners:
        s["proc"].start()

    # Iterate through all the Scanners and see if they died
    # Respawn them if they have.
    # (The Ravensticks occasionally suffer glib errors)
    try:
        while 1:
            for i, s in enumerate(scanners):
                
                # Wait on the join and then start it again if it died
                s["proc"].join(1)
                if not s["proc"].is_alive():
                    log_message = "{}: Caught error. Respawning".format(
                        s["devstring"])
                    if verbose:
                        print log_message
                    logging.warning(log_message)

                    # Add the cashed channel back to the list
                    channels.put(s["channel"].value)
                    s["channel"].value = 0

                    # Clean up from the Scanner crash
                    try:
                        s["dev"].sniffer_off()
                    except Exception as e:
                        log_message = "{}: Sniffer off error ({})".format(
                            s["devstring"],e)
                        if verbose:
                            print log_message
                        logging.warning(log_message)
                    try:
                        s["dev"].close()
                    except Exception as e:
                        log_message = "{}: Close error ({})".format(
                            s["devstring"],e)
                        if verbose:
                            print log_message
                        logging.warning(log_message)

                    # Resync the device and create another scanner
                    s["dev"] = create_device(
                        s["devstring"], verbose=verbose,
                        timeout=timeout, tries_limit=tries_limit)
                    s["proc"] = Scanner(
                        s["dev"], s["devstring"], s["channel"], channels,
                        verbose, currentGPS, s["kill"], output,
                        scanning_time, capture_time)

                    # Add the the list first in case start throws an error
                    # so we can kill/redo the new one
                    scanners[i] = s
                    scanners[i]["proc"].start()

    except KeyboardInterrupt:
        log_message = "doScan() ended by KeyboardInterrupt"
        if verbose:
            print log_message
        logging.info(log_message)
    except Exception as e:
        log_message = "doScan() caught non-Keyboard error: (%s)\n" % (e)
        log_message += traceback.format_exc()
        if verbose:
            print log_message
        logging.warning(log_message)
    finally:
        # Kill off all the children processes
        # aka prolicide
        for s in scanners:
            s["kill"].set()
        logging.debug(log_message)
        while not channels.empty():
            channels.get()

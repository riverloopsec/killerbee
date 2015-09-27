# This file is Copyright (c) 2010 by the GPSD project
# BSD terms apply: see the file COPYING in the distribution root for details.
#
import time, socket, sys, select
import logging

if sys.hexversion >= 0x2060000:
    import json			# For Python 2.6
else:
    import simplejson as json	# For Python 2.4 and 2.5

GPSD_PORT="2947"

class gpscommon:
    "Isolate socket handling and buffering from the protcol interpretation."
    def __init__(self, host="127.0.0.1", port=GPSD_PORT, verbose=0):
        self.sock = None        # in case we blow up in connect
        self.linebuffer = ""
        self.verbose = verbose
        self.connect(host, port)

    def connect(self, host, port):
        """Connect to a host on a given port.

        If the hostname ends with a colon (`:') followed by a number, and
        there is no port specified, that suffix will be stripped off and the
        number interpreted as the port number to use.
        """
        if not port and (host.find(':') == host.rfind(':')):
            i = host.rfind(':')
            if i >= 0:
                host, port = host[:i], host[i+1:]
            try: port = int(port)
            except ValueError:
                log_message = "nonnumeric port: {}".format(socket.error)
                logging.warning(log_message)
                raise socket.error, "nonnumeric port" 
        #if self.verbose > 0:
        #    print 'connect:', (host, port)
        msg = "getaddrinfo returns an empty list"
        self.sock = None
        for res in socket.getaddrinfo(host, port, 0, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            try:
                self.sock = socket.socket(af, socktype, proto)
                #if self.debuglevel > 0: print 'connect:', (host, port)
                self.sock.connect(sa)
            except socket.error, msg:
                log_message = "Connect fail: {},{}".format(host, port)
                logging.warning(log_message)
                #if self.debuglevel > 0: print 'connect fail:', (host, port)
                self.close()
                continue
            break
        if not self.sock:
            raise socket.error, msg

    def close(self):
        if self.sock:
            self.sock.close()
        self.sock = None

    def __del__(self):
        self.close()

    def waiting(self):
        "Return True if data is ready for the client."
        if self.linebuffer:
            return True
        (winput, woutput, wexceptions) = select.select((self.sock,), (), (), 0)
        return winput != []

    def read(self):
        "Wait for and read data being streamed from the daemon."
        if self.verbose > 1:
            sys.stderr.write("poll: reading from daemon...\n")
        eol = self.linebuffer.find('\n')
        if eol == -1:
            frag = self.sock.recv(4096)
            self.linebuffer += frag
            if self.verbose > 1:
                sys.stderr.write("poll: read complete.\n")
            if not self.linebuffer:
                if self.verbose > 1:
                    sys.stderr.write("poll: returning -1.\n")
                # Read failed
                return -1
            eol = self.linebuffer.find('\n')
            if eol == -1:
                if self.verbose > 1:
                    sys.stderr.write("poll: returning 0.\n")
                # Read succeeded, but only got a fragment
                return 0
        else:
            if self.verbose > 1:
                sys.stderr.write("poll: fetching from buffer.\n")

        # We got a line
        eol += 1
        self.response = self.linebuffer[:eol]
        self.linebuffer = self.linebuffer[eol:]

        # Can happen if daemon terminates while we're reading.
        if not self.response:
            return -1
        if self.verbose:
            sys.stderr.write("poll: data is %s\n" % repr(self.response))
        self.received = time.time()
        # We got a \n-terminated line
        return len(self.response)

    def send(self, commands):
        "Ship commands to the daemon."
        if not commands.endswith("\n"):
            commands += "\n"
        self.sock.send(commands)

WATCH_DISABLE	= 0x0000
WATCH_ENABLE	= 0x0001
WATCH_JSON	= 0x0002
WATCH_NMEA	= 0x0004
WATCH_RARE	= 0x0008
WATCH_RAW	= 0x0010
WATCH_SCALED	= 0x0020
WATCH_DEVICE	= 0x0040

class gpsjson(gpscommon):
    "Basic JSON decoding."
    def __iter__(self):
        return self

    def json_unpack(self, buf):
        def asciify(d):
            "De-Unicodify everything so we can copy dicts into Python objects."
            t = {}
            for (k, v) in d.items():
                ka = k.encode("ascii")
                if type(v) == type(u"x"):
                    va = v.encode("ascii")
                elif type(v) == type({}):
                    va = asciify(v)
                elif type(v) == type([]):
                    va = map(asciify, v)
                else:
                    va = v
                t[ka] = va
            return t
        self.data = dictwrapper(**asciify(json.loads(buf.strip(), encoding="ascii")))
        # Should be done for any other array-valued subobjects, too.
        if self.data["class"] == "SKY" and hasattr(self.data, "satellites"):
            self.data.satellites = map(lambda x: dictwrapper(**x), self.data.satellites)

    def stream(self, flags=0, outfile=None):
        "Control streaming reports from the daemon,"
        if flags & WATCH_DISABLE:
            arg = '?WATCH={"enable":false'
            if flags & WATCH_JSON:
                arg += ',"json":false'
            if flags & WATCH_NMEA:
                arg += ',"nmea":false'
            if flags & WATCH_RARE:
                arg += ',"raw":1'
            if flags & WATCH_RAW:
                arg += ',"raw":2'
            if flags & WATCH_SCALED:
                arg += ',"scaled":false'
        else: # flags & WATCH_ENABLE:
            arg = '?WATCH={"enable":true'
            if flags & WATCH_JSON:
                arg += ',"json":true'
            if flags & WATCH_NMEA:
                arg += ',"nmea":true'
            if flags & WATCH_RAW:
                arg += ',"raw":1'
            if flags & WATCH_RARE:
                arg += ',"raw":0'
            if flags & WATCH_SCALED:
                arg += ',"scaled":true'
            if flags & WATCH_DEVICE:
                arg += ',"device":"%s"' % outfile
        return self.send(arg + "}")

class dictwrapper:
    "Wrapper that yields both class and dictionary behavior,"
    def __init__(self, **ddict):
        self.__dict__ = ddict
    def get(self, k, d=None):
        return self.__dict__.get(k, d)
    def keys(self):
        return self.__dict__.keys()
    def __getitem__(self, key):
        "Emulate dictionary, for new-style interface."
        return self.__dict__[key]
    def __setitem__(self, key, val):
        "Emulate dictionary, for new-style interface."
        self.__dict__[key] = val
    def __contains__(self, key):
        return key in self.__dict__
    def __str__(self):
        return "<dictwrapper: " + str(self.__dict__) + ">"
    __repr__ = __str__

#
# Someday a cleaner Python iterface using this machiner will live here
#

# End

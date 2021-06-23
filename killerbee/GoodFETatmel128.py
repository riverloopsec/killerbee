# GoodFETclient to interface zigduino/atmel128 radio
# forked by bx from code by neighbor Travis Goodspeed
from .GoodFETAVR import GoodFETAVR # type: ignore # type: ignore
import sys, binascii, os, array, time, glob, struct # type: ignore

fmt = ("B", "<H", None, "<L")

class GoodFETatmel128rfa1(GoodFETAVR):
    ATMELRADIOAPP = 0x53
    autocrc = 0
    verbose = False
    connected = 0
    enable_AACK = False
    def serInit(self, port=None, timeout=2, attemptlimit=None):
        if port==None:
            port=os.environ.get("GOODFET");
        self.pyserInit(port, timeout, attemptlimit)

    def pyserInit(self, port, timeout, attemptlimit):
        """Open the serial port"""
        if self.connected == 0:
            if (not (attemptlimit == None)) and (attemptlimit <= 1):
                # it always takes at least 2 tries
                attemptlimit == 2

            # Make timeout None to wait forever, 0 for non-blocking mode.
            import serial; # type: ignore

            if os.name=='nt' and sys.version.find('64 bit')!=-1:
                print("WARNING: PySerial requires a 32-bit Python build in Windows.");

            if port is None and os.environ.get("GOODFET")!=None:
                glob_list = glob.glob(os.environ.get("GOODFET"));
                if len(glob_list) > 0:
                    port = glob_list[0];
                else:
                    port = os.environ.get("GOODFET");
            if port is None:
                glob_list = glob.glob("/dev/tty.usbserial*");
                if len(glob_list) > 0:
                    port = glob_list[0];
            if port is None:
                glob_list = glob.glob("/dev/ttyUSB*");
                if len(glob_list) > 0:
                    port = glob_list[0];
            if port is None:
                glob_list = glob.glob("/dev/ttyU0");
                if len(glob_list) > 0:
                    port = glob_list[0];
            if port is None and os.name=='nt':
                from scanwin32 import winScan; # type: ignore
                scan=winScan();
                for order,comport,desc,hwid in sorted(scan.comports()):
                    try:
                        if hwid.index('FTDI')==0:
                            port=comport;
                    except:
                        #Do nothing.
                        a=1;

            baud=115200;

            self.serialport = serial.Serial(
                port,
                baud,
                parity = serial.PARITY_NONE,
                timeout=timeout
                )

            self.verb=0;
            self.data=""
            attempts=0;
            self.connected=0;

            while self.connected==0:
                self.serialport.setDTR(False)
                while self.verb!=0x7F or self.data!="http://goodfet.sf.net/":
                    if attemptlimit is not None and attempts >= attemptlimit:
                        return

                    attempts=attempts+1;
                    self.readcmd(); #Read the first command.
                    if self.verbose:
                        print("Got %02x,%02x:'%s'" % (self.app,self.verb,self.data));

                #Here we have a connection, but maybe not a good one.
                for foo in range(1,30):
                    time.sleep(1)
                    if not self.monitorecho():
                        self.connected = 0
                        if self.verbose:
                            print("Comm error on try %i." % (foo))
                    else:
                        self.connected = 1
                        break
            if self.verbose:
                print("Connected after %02i attempts." % attempts);
            self.serialport.setTimeout(12);

    def serClose(self):
        self.connected = 0
        self.serialport.close()


    def writecmd(self, app, verb, count=0, data=[]):
        """Write a command and some data to the GoodFET."""
        self.serialport.write(chr(app));
        self.serialport.write(chr(verb));
        if self.verbose:
            print("Tx: ( 0x%02x, 0x%02x, %d )" % ( app, verb, count ))
        if count > 0:
            if(isinstance(data,list)):
                old = data
                data = []
                for i in range(0,count):
                    data += chr(old[i]);
            outstr=''.join(data);

        #little endian 16-bit length
            count = len(outstr)
        self.serialport.write(chr(count&0xFF));
        self.serialport.write(chr(count>>8));
        if count > 0:
            if self.verbose:
                print("sending: %s" %outstr.encode("hex"))
            self.serialport.write(outstr);


        if not self.besilent:
            out = self.readcmd()
            if out and self.verbose:
                print("read: " + out.encode("hex"))
            return out
        else:
            return None

    def readcmd(self):

        """Read a reply from the GoodFET."""
        app = self.serialport.read(1)

        if len(app) < 1:
            if self.verbose:
                print("Rx: None")

            self.app = 0
            self.verb = 0
            self.count = 0
            self.data = ""
            return

        self.app=ord(app);

        v = self.serialport.read(1);
        if v:
            self.verb = ord(v)
        else:
            self.verb = 0
            
        c1 = self.serialport.read(1)
        c2 = self.serialport.read(1)
        if (c1 and c2):
            self.count= ord(c1) + (ord(c2)<<8)
        else:
            self.count = 0
        if self.verbose:
            print("Rx: ( 0x%02x, 0x%02x, %i )" % ( self.app, self.verb, self.count ))

        #Debugging string; print, but wait.
        if self.app==0xFF:
            if self.verb==0xFF:
                print("# DEBUG %s" % self.serialport.read(self.count))
            elif self.verb==0xFE:
                print("# DEBUG 0x%x" % struct.unpack(fmt[self.count-1], self.serialport.read(self.count))[0])
            elif self.verb==0xFD:
                        #Do nothing, just wait so there's no timeout.
                print("# NOP.");
            return ""
        else:
            self.data=self.serialport.read(self.count);
            return self.data;

    def RF_setchannel(self, chan):
        if (chan < 11) or (chan > 26):
            print("Channel out of range")
        else:
            self.poke(0x8, chan)

    def peek(self,reg,bytes=1):
        """Read a Register. """
        #Automatically calibrate the len.
        if bytes != 1:
            print("Warning, currently cannot poke more than 1 byte")
            bytes = 1
        data = [reg, 0, bytes%255, bytes>>8] #+ ([0]*bytes)
        self.data = None
        self.writecmd(self.ATMELRADIOAPP,0x02,len(data),data);
        toret=0;
        if self.data:
            #for i in range(0,bytes):
            #    toret=toret|(ord(self.data[i+1])<<(8*i));
            #return toret;
            # right now only works with a byte of data
            return ord(self.data)
        else:
            return -1

    def poke(self,reg,val,bytes=1): # todo, support >1 byte
        """Write an Register."""
        data = [reg, 0] #+ ([0]*bytes)
        data=[reg, 0]
        if bytes != 1:
            print("Warning, currently cannot poke more than 1 byte")
            bytes = 1
        for i in range(0,bytes):
            data=data+[(val>>(8*i))&0xFF];

        self.writecmd(self.ATMELRADIOAPP,0x03,len(data),data);
        newval = self.peek(reg,bytes)
        if newval!=val:
            print("Warning, failed to set r%02x=%02x, got %02x." %(
                reg,
                val,
                newval));

        return;

    def setup(self):
        self.RF_setup()

    def RF_setup(self):
        self.writecmd(self.ATMELRADIOAPP, 0x10, 0, None)

    def RF_rxpacket(self):
        """Get a packet from the radio.  Returns None if none is waiting."""
        #doto: check if packet has arrived, flush if not new
        self.writecmd(self.ATMELRADIOAPP, 0x80, 0, None)
        data=self.data;
        self.packetlen = len(data)
        if (self.packetlen > 0):
            return data;
        else:
            return None

    def RF_txpacket(self, payload):
        if type(payload) == list: #convert to string
            import array # type: ignore
            payload = array.array('B', payload).tostring()
        self.writecmd(self.ATMELRADIOAPP, 0x81, len(payload), payload)


    def RF_getrssi(self):
        """Returns the received signal strength"""
        base = -90
        val = self.peek(0x7) & 0x7f # read rssi bits
        if val == 0:
            return base - 1
        elif val < 0x53:
            return val + base
        else:
            return 0x53 + base

    def RF_enable_AACK(self, enable = True):
        if (enable and (not self.enable_AACK)):
            self.enable_AACK = True
            self.writecmd(self.ATMELRADIOAPP, 0x84)
        elif ((not enable) and self.enable_AACK):
            self.enable_AACK = False
            self.writecmd(self.ATMELRADIOAPP, 0x85)


    def RF_autocrc(self, autocrc=1):
        self.autocrc = autocrc
        if autocrc:
            self.writecmd(self.ATMELRADIOAPP, 0x86)
        else:
            self.writecmd(self.ATMELRADIOAPP, 0x87)


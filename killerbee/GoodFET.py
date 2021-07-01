#!/usr/bin/env python
# GoodFET Client Library
# 
# (C) 2009 Travis Goodspeed <travis at radiantmachines.com>
#
# This code is being rewritten and refactored.  You've been warned!

from typing import Dict, Union, Optional, Any, List, Tuple

import sys 
import time
import string
import io
import struct
import glob
import os; 
import sqlite3; 

fmt: Tuple[str, str, str, str] = ("B", "<H", "<3S", "<L")

def getClient(name="GoodFET") -> Any:
    import GoodFET, GoodFETCC, GoodFETAVR, GoodFETSPI, GoodFETMSP430, GoodFETNRF, GoodFETCCSPI; # type: ignore
    if(name=="GoodFET" or name=="monitor"): return GoodFET.GoodFET();
    elif name=="cc" or name=="cc51": return GoodFETCC.GoodFETCC();
    elif name=="cc2420" or name=="ccspi": return GoodFETCCSPI.GoodFETCCSPI();
    elif name=="avr": return GoodFETAVR.GoodFETAVR();
    elif name=="spi": return GoodFETSPI.GoodFETSPI();
    elif name=="msp430": return GoodFETMSP430.GoodFETMSP430();
    elif name=="nrf": return GoodFETNRF.GoodFETNRF();
    
    print("Unsupported target: {}".format(name))
    sys.exit(0);

class SymbolTable:
    """GoodFET Symbol Table"""
    db: Any =sqlite3.connect(":memory:");
    
    def __init__(self, *args: Any, **kargs: Any) -> None:
        self.db.execute("create table if not exists symbols(adr,name,memory,size,comment);");

    def get(self, name: str) -> Any:
        self.db.commit();

        c: Any =self.db.cursor();

        try:
            c.execute("select adr,memory from symbols where name=?",(name,));
            for row in c:
                sys.stdout.flush();
                return row[0];
        except:# sqlite3.OperationalError:
            pass

        return eval(name);

    def define(self, adr: Any, name: str, comment: str="", memory: str="vn", size: int=16) -> None:
        self.db.execute("insert into symbols(adr,name,memory,size,comment)"
                        "values(?,?,?,?,?);", (
                adr,name,memory,size,comment));

class GoodFET:
    """GoodFET Client Library"""

    besilent: int = 0;
    app: int = 0;
    verb: int = 0;
    count: int = 0;
    data: bytearray = bytearray([]);
    verbose: bool = False
    
    GLITCHAPP: int = 0x71;
    MONITORAPP: int = 0x00;
    symbols: SymbolTable = SymbolTable();
    
    def __init__(self, *args: Any, **kargs: Any) -> None:
        self.data: bytearray = bytearray([]); 

    def getConsole(self) -> Any:
        from GoodFETConsole import GoodFETConsole; # type: ignore
        return GoodFETConsole(self);

    def name2adr(self, name: str) -> str:
        return self.symbols.get(name);

    def timeout(self) -> None:
        print("timeout\n");

    def serInit(self, port: Optional[str]=None, timeout: int=2, attemptlimit: Optional[int]=None) -> None:
        """Open a serial port of some kind."""
        import re; 
        
        if port is None:
            port = os.environ.get("GOODFET");

        self.pyserInit(port, timeout, attemptlimit);

    def pyserInit(self, port, timeout, attemptlimit):
        """Open the serial port"""
        import serial; # type: ignore
        fixserial: bool = False;
        
        if os.name == 'nt' and sys.version.find('64 bit') != -1:
            print("WARNING: PySerial requires a 32-bit Python build in Windows.");
        
        if port is None and os.environ.get("GOODFET") != None:
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
                    pass
        
        baud: int = 115200;

        if(os.environ.get("platform")=='arduino' or os.environ.get("board")=='arduino'):
            baud=19200 #Slower, for now.

        self.serialport = serial.Serial(
            port,
            baud,
            parity = serial.PARITY_NONE,
            timeout=timeout
        )
        
        self.verb = 0;
        attempts: int = 0;
        connected = False;
        while connected is False:
            while self.verb != 0x7F or self.data != b"http://goodfet.sf.net/":

                if attemptlimit is not None and attempts >= attemptlimit:
                    return

                elif attempts == 2 and os.environ.get("board") != 'telosb':
                    print("See the GoodFET FAQ about missing info flash.");
                    self.serialport.timeout = 0.2;

                elif attempts == 100:
                    print("Tried 100 times to connect and failed.")
                    print("Continuing to try forever.", end='')
                    self.verbose = True   # Something isn't going right, give the user more info

                elif attempts > 100 and attempts % 10 == 0:
                    print('.', end='')

                #TelosB reset, prefer software to I2C SPST Switch.
                if (os.environ.get("board") == 'telosb'):
                    self.telosBReset();

                elif (os.environ.get("board") == 'z1'):
                    self.bslResetZ1(invokeBSL=0);

                elif (os.environ.get("board")=='apimote1') or (os.environ.get("board")=='apimote'):
                    self.serialport.setRTS(1);
                    self.serialport.setDTR(1);
                    self.serialport.setRTS(0);
                else:
                    self.serialport.setRTS(1);
                    self.serialport.setDTR(1);
                    self.serialport.setDTR(0);
                
                attempts = attempts + 1;
                self.readcmd(); #Read the first command.

                if self.verb!=0x7f:
                    self.readcmd();

            connected=True;

        if attempts >= 100:
            print("")   # Add a newline
            olds: str = self.infostring();
            clocking: Any = self.monitorclocking();

            for foo in range(1,30):
                if not self.monitorecho():
                    if self.verbose:
                        print("Comm error on {0} try, resyncing out of {1}.".format(foo ,clocking));
                    connected=False;
                    break;

        if self.verbose: 
            print("Connected after {} attempts.".format(attempts));

        self.mon_connected();
        self.serialport.timeout = 12;

    def serClose(self) -> None:
        self.serialport.close();

    def telosSetSCL(self, level: int) -> None:
        '''Helper function for support of the TelosB platform.'''
        self.serialport.setRTS(not level)

    def telosSetSDA(self, level: int) -> None:
        '''Helper function for support of the TelosB platform.'''
        self.serialport.setDTR(not level)

    def telosI2CStart(self) -> None:
        '''Helper function for support of the TelosB platform.'''
        self.telosSetSDA(1)
        self.telosSetSCL(1)
        self.telosSetSDA(0)

    def telosI2CStop(self) -> None:
        '''Helper function for support of the TelosB platform.'''
        self.telosSetSDA(0)
        self.telosSetSCL(1)
        self.telosSetSDA(1)

    def telosI2CWriteBit(self, bit: int) -> None:
        '''Helper function for support of the TelosB platform.'''
        self.telosSetSCL(0)
        self.telosSetSDA(bit)
        time.sleep(2e-6)
        self.telosSetSCL(1)
        time.sleep(1e-6)
        self.telosSetSCL(0)

    def telosI2CWriteByte(self, byte: int) -> None:
        '''Helper function for support of the TelosB platform.'''
        self.telosI2CWriteBit( byte & 0x80 );
        self.telosI2CWriteBit( byte & 0x40 );
        self.telosI2CWriteBit( byte & 0x20 );
        self.telosI2CWriteBit( byte & 0x10 );
        self.telosI2CWriteBit( byte & 0x08 );
        self.telosI2CWriteBit( byte & 0x04 );
        self.telosI2CWriteBit( byte & 0x02 );
        self.telosI2CWriteBit( byte & 0x01 );
        self.telosI2CWriteBit( 0 );  # "acknowledge"

    def telosI2CWriteCmd(self, addr: int, cmdbyte: int) -> None:
        '''Helper function for support of the TelosB platform.'''
        self.telosI2CStart()
        self.telosI2CWriteByte( 0x90 | (addr << 1) )
        self.telosI2CWriteByte( cmdbyte )
        self.telosI2CStop()

    def bslResetZ1(self, invokeBSL: int=0) -> None:
        '''
        Helper function for support of the Z1 mote platform.
        Applies BSL entry sequence on RST/NMI and TEST/VPP pins.
        By now only BSL mode is accessed.
        @type  invokeBSL: Integer
        @param invokeBSL: 1 for a complete sequence, or 0 to only access RST/NMI pin
        Applies BSL entry sequence on RST/NMI and TEST/VPP pins
        Parameters:
            invokeBSL = 1: complete sequence
            invokeBSL = 0: only RST/NMI pin accessed
            
        By now only BSL mode is accessed
        '''

        if invokeBSL:
            time.sleep(0.1)
            self.writepicROM(0xFF, 0xFF)
            time.sleep(0.1)
        else:
            time.sleep(0.1)
            self.writepicROM(0xFF, 0xFE)
            time.sleep(0.1)

    def writepicROM(self, address: int, data: int) -> int:
        ''' Writes data to @address'''

        for i in range(7,-1,-1):
            self.picROMclock((address >> i) & 0x01)

        self.picROMclock(0)

        recbuf: int = 0

        for i in range(7,-1,-1):
            s: int = ((data >> i) & 0x01)
            if i < 1:
                r: bool = not self.picROMclock(s, True)
            else:
                r = not self.picROMclock(s)
            recbuf = (recbuf << 1) + r

        self.picROMclock(0, True)
        return recbuf

    def readpicROM(self, address: int) -> int:
        ''' reads a byte from @address'''
        for i in range(7,-1,-1):
            self.picROMclock((address >> i) & 0x01)

        self.picROMclock(1)

        recbuf: int = 0
        r: int = 0

        for i in range(7,-1,-1):
            r = self.picROMclock(0)
            recbuf = (recbuf << 1) + r

        self.picROMclock(r)
        return recbuf
        
    #This seems more reliable when slowed.
    def picROMclock(self, masterout: int, slow: bool=True) -> int:
        self.serialport.setRTS(masterout)
        self.serialport.setDTR(1)
        self.serialport.setDTR(0)

        if slow:
            time.sleep(0.02)

        return self.serialport.getCTS()

    def picROMfastclock(self, masterout: int) -> int:
        return self.picROMclock(masterout, False)

    def telosBReset(self, invokeBSL: int=0) -> None:
        # "BSL entry sequence at dedicated JTAG pins"
        # rst !s0: 0 0 0 0 1 1
        # tck !s1: 1 0 1 0 0 1
        #   s0|s1: 1 3 1 3 2 0

        # "BSL entry sequence at shared JTAG pins"
        # rst !s0: 0 0 0 0 1 1
        # tck !s1: 0 1 0 1 1 0
        #   s0|s1: 3 1 3 1 0 2

        if invokeBSL:
            self.telosI2CWriteCmd(0,1)
            self.telosI2CWriteCmd(0,3)
            self.telosI2CWriteCmd(0,1)
            self.telosI2CWriteCmd(0,3)
            self.telosI2CWriteCmd(0,2)
            self.telosI2CWriteCmd(0,0)
        else:
            self.telosI2CWriteCmd(0,3)
            self.telosI2CWriteCmd(0,2)

        # This line was not defined inside the else: block, not sure where it
        # should be however
        self.telosI2CWriteCmd(0,0)
        time.sleep(0.250)       #give MSP430's oscillator time to stabilize
        self.serialport.flushInput()  #clear buffers

    def writecmd(self, app: int, verb: int, count: int=0, data: Optional[bytes]=None)-> bytearray:
        """Write a command and some data to the GoodFET."""
        self.serialport.write(bytearray([app, verb]))

        self.serialport.write(bytearray([count & 0xFF, count >> 8]))

        if self.verbose:
            print("Tx: ( {0}, {1}, {2} )".format( app, verb, count ))
        
        if count != 0:
            if data is not None:
                out: bytearray = bytearray(data[:count])

            self.serialport.write(out)

        if not self.besilent:
            return self.readcmd()
        else:
            return bytearray([0])

    def readcmd(self) -> bytearray:
        """Read a reply from the GoodFET."""
        while 1:
            try:
                self.app = ord(self.serialport.read(1));
                self.verb = ord(self.serialport.read(1));
                
                #Fixes an obscure bug in the TelosB.
                if self.app == 0x00:
                    while self.verb == 0x00:
                        self.verb = ord(self.serialport.read(1));
               
                self.count=(
                    ord(self.serialport.read(1)) + (ord(self.serialport.read(1))<<8)
                );

                if self.verbose:
                    print("Rx: ( {0}, {1}, {2} )".format( self.app, self.verb, self.count ))
            
                #Debugging string; print, but wait.
                if self.app == 0xFF:
                    if self.verb == 0xFF:
                        print("# DEBUG %s" % self.serialport.read(self.count))

               	    elif self.verb==0xFE:
                        print("# DEBUG 0x{:x}".format(struct.unpack(fmt[self.count-1], self.serialport.read(self.count))[0]))
                    elif self.verb==0xFD:
                        print("# NOP.")
                        
                    sys.stdout.flush();

                else:
                    self.data = bytearray(self.serialport.read(self.count))
                    return self.data;

            except TypeError:
                if self.connected:
                    print("Warning: waiting for serial read timed out (most likely).")
                    #sys.exit(-1)
                return self.data;

    #Glitching stuff.
    def glitchApp(self,app):
        """Glitch into a device by its application."""
        self.data=[app&0xff];
        self.writecmd(self.GLITCHAPP,0x80,1,self.data);
        #return ord(self.data[0]);
    def glitchVerb(self,app,verb,data):
        """Glitch during a transaction."""
        if data==None: data=[];
        self.data=[app&0xff, verb&0xFF]+data;
        self.writecmd(self.GLITCHAPP,0x81,len(self.data),self.data);
        #return ord(self.data[0]);
    def glitchstart(self):
        """Glitch into the AVR application."""
        self.glitchVerb(self.APP,0x20,None);
    def glitchstarttime(self):
        """Measure the timer of the START verb."""
        return self.glitchTime(self.APP,0x20,None);
    def glitchTime(self,app,verb,data):
        """Time the execution of a verb."""
        if data==None: data=[];
        self.data=[app&0xff, verb&0xFF]+data;
        print("Timing app %02x verb %02x." % (app,verb));
        self.writecmd(self.GLITCHAPP,0x82,len(self.data),self.data);
        time=ord(self.data[0])+(ord(self.data[1])<<8);
        print("Timed to be %i." % time);
        return time;
    def glitchVoltages(self,low=0x0880, high=0x0fff):
        """Set glitching voltages. (0x0fff is max.)"""
        self.data=[low&0xff, (low>>8)&0xff,
                   high&0xff, (high>>8)&0xff];
        self.writecmd(self.GLITCHAPP,0x90,4,self.data);
        #return ord(self.data[0]);
    def glitchRate(self,count=0x0800):
        """Set glitching count period."""
        self.data=[count&0xff, (count>>8)&0xff];
        self.writecmd(self.GLITCHAPP,0x91,2,
                      self.data);
        #return ord(self.data[0]);
    
    #Monitor stuff
    def silent(self,s=0):
        """Transmissions halted when 1."""
        self.besilent=s;
        print("besilent is %i" % self.besilent);
        self.writecmd(0,0xB0,1,[s]);
    connected=0;
    def mon_connected(self):
        """Announce to the monitor that the connection is good."""
        self.connected=1;
        self.writecmd(0,0xB1,0,[]);
    def out(self,byte):
        """Write a byte to P5OUT."""
        self.writecmd(0,0xA1,1,[byte]);
    def dir(self,byte):
        """Write a byte to P5DIR."""
        self.writecmd(0,0xA0,1,[byte]);
    def call(self,adr):
        """Call to an address."""
        self.writecmd(0,0x30,2,
                      [adr&0xFF,(adr>>8)&0xFF]);
    def execute(self,code):
        """Execute supplied code."""
        self.writecmd(0,0x31,2,#len(code),
                      code);
    def MONpeek8(self,address):
        """Read a byte of memory from the monitor."""
        self.data=[address&0xff,address>>8];
        self.writecmd(0,0x02,2,self.data);
        #self.readcmd();
        return ord(self.data[0]);
    def MONpeek16(self,address):
        """Read a word of memory from the monitor."""
        return self.MONpeek8(address)+(self.MONpeek8(address+1)<<8);
    def peek(self,address):
        """Read a word of memory from the monitor."""
        return self.MONpeek8(address)+(self.MONpeek8(address+1)<<8);
    def eeprompeek(self,address):
        """Read a word of memory from the monitor."""
        print("EEPROM peeking not supported for the monitor.");
        #return self.MONpeek8(address)+(self.MONpeek8(address+1)<<8);
    def peekbysym(self,name):
        """Read a value by its symbol name."""
        #TODO include memory in symbol.
        reg=self.symbols.get(name);
        return self.peek8(reg,"data");
    def pokebysym(self,name,val):
        """Write a value by its symbol name."""
        #TODO include memory in symbol.
        reg=self.symbols.get(name);
        return self.pokebyte(reg,val);
    def pokebyte(self,address,value,memory="vn"):
        """Set a byte of memory by the monitor."""
        self.data=[address&0xff,address>>8,value];
        self.writecmd(0,0x03,3,self.data);
        return ord(self.data[0]);
    def poke16(self,address,value):
        """Set a word of memory by the monitor."""
        self.MONpoke16(address,value);
    def MONpoke16(self,address,value):
        """Set a word of memory by the monitor."""
        self.pokebyte(address,value&0xFF);
        self.pokebyte(address,(value>>8)&0xFF);
        return value;
    def setsecret(self,value):
        """Set a secret word for later retreival.  Used by glitcher."""
        #self.eeprompoke(0,value);
        #self.eeprompoke(1,value);
        print("Secret setting is not yet suppored for this target.");
        print("Aborting.");
        
    def getsecret(self):
        """Get a secret word.  Used by glitcher."""
        #self.eeprompeek(0);
        print("Secret getting is not yet suppored for this target.");
        print("Aborting.");
        sys.exit();
    
    def dumpmem(self,begin,end):
        i=begin;
        while i<end:
            print("%04x %04x" % (i, self.MONpeek16(i)));
            i+=2;
    def monitor_ram_pattern(self):
        """Overwrite all of RAM with 0xBEEF."""
        self.writecmd(0,0x90,0,self.data);
        return;
    def monitor_ram_depth(self):
        """Determine how many bytes of RAM are unused by looking for 0xBEEF.."""
        self.writecmd(0,0x91,0,self.data);
        return ord(self.data[0])+(ord(self.data[1])<<8);
    
    #Baud rates
    baudrates=[115200, 
               9600,
               19200,
               38400,
               57600,
               115200];
    def setBaud(self,baud):
        """Change the baud rate.  TODO fix this."""
        rates=self.baudrates;
        self.data=[baud];
        print("Changing FET baud.")
        self.serialport.write(bytearray([0x00, 0x80, 1, baud]))

        print("Changed host baud.")
        self.serialport.setBaudrate(rates[baud]);
        time.sleep(1);
        self.serialport.flushInput()
        self.serialport.flushOutput()
        
        print("Baud is now %i." % rates[baud]);
        return;
    def readbyte(self):
        return ord(self.serialport.read(1));
    def findbaud(self):
        for r in self.baudrates:
            print("\nTrying %i" % r);
            self.serialport.setBaudrate(r);
            #time.sleep(1);
            self.serialport.flushInput()
            self.serialport.flushOutput()
            
            for i in range(1,10):
                self.readbyte();
            
            print("Read %02x %02x %02x %02x" % (
                self.readbyte(),self.readbyte(),self.readbyte(),self.readbyte()));

    def monitortest(self):
        """Self-test several functions through the monitor."""
        print("Performing monitor self-test.");
        self.monitorclocking();
        for f in range(0,3000):
            a=self.MONpeek16(0x0c00);
            b=self.MONpeek16(0x0c02);
            if a!=0x0c04 and a!=0x0c06:
                print("ERROR Fetched %04x, %04x" % (a,b));
            self.pokebyte(0x0021,0); #Drop LED
            if self.MONpeek8(0x0021)!=0:
                print("ERROR, P1OUT not cleared.");
            self.pokebyte(0x0021,1); #Light LED
            if not self.monitorecho():
                print("Echo test failed.");
        print("Self-test complete.");
        self.monitorclocking();

    def monitorecho(self):
        data=b"The quick brown fox jumped over the lazy dog.";
        self.writecmd(self.MONITORAPP,0x81,len(data),data);
        if self.data!=data:
            if self.verbose:
                print("Comm error recognized by monitorecho(), got:\n%s" % self.data);
            return 0;
        return 1;

    def monitor_info(self):
        print("GoodFET with %s MCU" % self.infostring());
        print("Clocked at %s" % self.monitorclocking());
        return 1;

    def testleds(self):
        print("Flashing LEDs")
        self.writecmd(self.MONITORAPP,0xD0,0,b"")
        try:
            print("Flashed %d LED." % ord(self.data))
        except:
            print("Unable to process response:", self.data)
        return 1

    def monitor_list_apps(self, full=False): 
        self.monitor_info()
        old_value = self.besilent
        self.besilent = True    # turn off automatic call to readcmd
        self.writecmd(self.MONITORAPP, 0x82, 1, [int(full)]);
        self.besilent = old_value
        
        # read the build date string 
        self.readcmd()
        print("Build Date: %s" % self.data)
        print("Firmware apps:")
        while True:
            self.readcmd()
            if self.count == 0:
                break
            print(self.data)
        return 1;

    def monitorclocking(self):
        """Return the 16-bit clocking value."""
        return "0x%04x" % self.monitorgetclock();
    
    def monitorsetclock(self,clock):
        """Set the clocking value."""
        self.MONpoke16(0x56, clock);
    def monitorgetclock(self):
        """Get the clocking value."""
        if(os.environ.get("platform")=='arduino' or os.environ.get("board")=='arduino'):
            return 0xDEAD;
        #Check for MSP430 before peeking this.
        return self.MONpeek16(0x56);

    # The following functions ought to be implemented in
    # every client.    
    def infostring(self):
        if(os.environ.get("platform")=='arduino' or os.environ.get("board")=='arduino'):
            #TODO implement in the ardunio client and remove special case from here
            return "Arduino";
        else:
            a=self.MONpeek8(0xff0);
            b=self.MONpeek8(0xff1);
            return "%02x%02x" % (a,b);
    def lock(self):
        print("Locking Unsupported.");
    def erase(self):
        print("Erasure Unsupported.");
    def setup(self):
        return;
    def start(self):
        return;
    def test(self):
        print("Unimplemented.");
        return;
    def status(self):
        print("Unimplemented.");
        return;
    def halt(self):
        print("Unimplemented.");
        return;
    def resume(self):
        print("Unimplemented.");
        return;
    def getpc(self):
        print("Unimplemented.");
        return 0xdead;
    def flash(self,file):
        """Flash an intel hex file to code memory."""
        print("Flash not implemented.");
    def dump(self,file,start=0,stop=0xffff):
        """Dump an intel hex file from code memory."""
        print("Dump not implemented.");
    def peek32(self,address, memory="vn"):
        """Peek 32 bits."""
        return (self.peek16(address,memory)+
                (self.peek16(address+2,memory)<<16));
    def peek16(self,address, memory="vn"):
        """Peek 16 bits of memory."""
        return (self.peek8(address,memory)+
                (self.peek8(address+1,memory)<<8));
    def peek8(self,address, memory="vn"):
        """Peek a byte of memory."""
        return self.MONpeek8(address); #monitor
    def peekblock(self,address,length,memory="vn"):
        """Return a block of data."""
        data=list(range(0,length));
        for foo in range(0,length):
            data[foo]=self.peek8(address+foo,memory);
        return data;
    def pokeblock(self,address,bytes,memory="vn"):
        """Poke a block of a data into memory at an address."""
        for foo in bytes:
            self.pokebyte(address,foo,memory);
            address=address+1;
        return;
    def loadsymbols(self):
        """Load symbols from a file."""
        return;


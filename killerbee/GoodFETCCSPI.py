#!/usr/bin/env python
# GoodFET Chipcon RF Radio Client
# 
# (C) 2009, 2012 Travis Goodspeed <travis at radiantmachines.com>
#
# This code is being rewritten and refactored.  You've been warned!

from typing import Optional, Any

import sys, time, string, io, struct, glob, os; # type: ignore

from .GoodFET import GoodFET; # type: ignore
from .kbutils import bytearray_to_bytes # type: ignore

class GoodFETCCSPI(GoodFET):
    CCSPIAPP=0x51;
    CCversions={0x233d: "CC2420",
                }
    def setup(self):
        """Move the FET into the CCSPI application."""
        self.writecmd(self.CCSPIAPP,0x10,0,self.data); #CCSPI/SETUP
        
        #Set up the radio for ZigBee
        self.strobe(0x01);       #SXOSCON
        self.strobe(0x01);       #SXOSCON ##YL: idk why but needed to enable ram poking
        self.strobe(0x02);       #SCAL 
        self.poke(0x11, 0x0AC2 & (~0x0800)); #MDMCTRL0, promiscuous
        self.poke(0x12, 0x0500); #MDMCTRL1
        self.poke(0x1C, 0x007F); #IOCFG0
        self.poke(0x19, 0x01C4); #SECCTRL0, disabling crypto
        #self.poke(0x19, 0x0204); #SECCTRL0, as seen elsewhere.
        #self.RF_setsync();
        
    def ident(self):
        return self.peek(0x1E); #MANFIDL
    def identstr(self):
        manfidl=self.peek(0x1E);
        #manfidh=self.peek(0x1f);
        try:
            return "%s" % (self.CCversions[manfidl]); 
        except:
            return "Unknown0x%04x" % manfidl;
    def trans8(self,byte):
        """Read and write 8 bits by CCSPI."""
        data=self.CCSPItrans([byte]);
        return data[0];
    
    def trans(self,data):
        """Exchange data by CCSPI."""
        self.data=data;
        self.writecmd(self.CCSPIAPP,0x00,len(data),data);
        return self.data;
    def strobe(self,reg=0x00):
        """Strobes a strobe register, returning the status."""
        data=[reg];
        self.trans(data);
        return self.data[0];
    def CC_RFST_IDLE(self):
        """Switch the radio to idle mode, clearing overflows and errors."""
        self.strobe(0x06); #SRXOFF
    def CC_RFST_TX(self):
        """Switch the radio to TX mode."""
        self.strobe(0x04);  #0x05 for CCA
    def CC_RFST_RX(self):
        """Switch the radio to RX mode."""
        self.strobe(0x03); #RX ON
    def CC_RFST_CAL(self):
        """Calibrate strobe the radio."""
        self.strobe(0x02); #RX Calibrate
    def CC_RFST(self,state=0x00):
        self.strobe(state);
        return;
    def peek(self,reg,bytes=2):
        """Read a CCSPI Register.  For long regs, result is flipped."""
        
        #Reg is ORed with 0x40 by the GoodFET.
        data=[reg,0,0];
        
        #Automatically calibrate the len.
        bytes=2;
       
        resp: Optional[bytearray] = self.writecmd(self.CCSPIAPP,0x02,len(data),data);

        if len(resp) > 1: 
            try: 
                toret = resp[2] + (resp[1]<<8); 
            except IndexError:
                try:
                    toret = resp[1] << 8
                except IndexError:
                    toret = 0
        else:
            toret = 0
            
        return toret;

    def poke(self,reg,val,bytes=2):
        """Write a CCSPI Register."""
        data=[reg,(val>>8)&0xFF,val&0xFF];
        self.writecmd(self.CCSPIAPP,0x03,len(data),data);
        if self.peek(reg,bytes)!=val and reg!=0x18:
            print("Warning, failed to set r%02x=0x%04x, got %02x." %(
                reg,
                val,
                self.peek(reg,bytes)));
            return False;
        return True;
    
    def status(self):
        """Read the status byte."""
        statusbits={0x80: "?",
                    0x40: "XOSC16M_STABLE",
                    0x20: "TX_UNDERFLOW",
                    0x10: "ENC_BUSY",
                    0x08: "TX_ACTIVE",
                    0x04: "LOCK",
                    0x02: "RSSI_VALID",
                    0x01: "?"};
        status=self.strobe(0x00);
        i=1;
        str="";
        while i<0x100:
            if status&i:
               str="%s %s" % (statusbits[i],str);
            i*=2;
        return str;
    
    #Radio stuff begins here.
    def RF_setenc(self,code="802.15.4"):
        """Set the encoding type."""
        return code;
    def RF_getenc(self):
        """Get the encoding type."""
        return "802.15.4";
    def RF_getrate(self):
        return 0;
    def RF_setrate(self,rate=0):
        return 0;
    def RF_getsync(self):
        return self.peek(0x14);
    def RF_setsync(self,sync=0xa70F):
        """Set the SYNC preamble.
        Use 0xA70F for 0xA7."""
        self.poke(0x14,sync);
        return;
    
    def RF_setkey(self,key):
        """Sets the first key for encryption to the given argument."""
        print("ERROR: Forgot to set the key.");
        
        return;
    def RF_setnonce(self,key):
        """Sets the first key for encryption to the given argument."""
        print("ERROR: Forgot to set the nonce.");
        
        return;
    
    def RF_setfreq(self,frequency):
        """Set the frequency in Hz."""
        mhz=frequency/1000000;
        #fsctrl=0x8000; #
        fsctrl=self.peek(0x18)&(~0x3FF);
        fsctrl=fsctrl+int(mhz-2048)
        self.poke(0x18,fsctrl);
        #self.CC_RFST_IDLE();
        self.strobe(0x02);#SCAL
        time.sleep(0.01);
        self.strobe(0x03);#SRXON
    def RF_getfreq(self):
        """Get the frequency in Hz."""
        fsctrl=self.peek(0x18);
        mhz=2048+(fsctrl&0x3ff)
        return mhz*1000000;
    def RF_setchan(self,channel):
        """Set the ZigBee/802.15.4 channel number."""
        if channel < 11 or channel > 26:
            print("Only 802.15.4 channels 11 to 26 are currently supported.");
        else:
            self.RF_setfreq( ( (channel-11)*5 + 2405 ) * 1000000 );
    def RF_getsmac(self):
        """Return the source MAC address."""
        return 0xdeadbeef;
    def RF_setsmac(self,mac):
        """Set the source MAC address."""
        return 0xdeadbeef;
    def RF_gettmac(self):
        """Return the target MAC address."""
        return 0xdeadbeef;
    def RF_settmac(self,mac):
        """Set the target MAC address."""
        return 0xdeadbeef;
    def RF_getrssi(self):
        """Returns the received signal strength, with a weird offset."""
        rssival=self.peek(0x13)&0xFF; #raw RSSI register
        return rssival^0x80;

    def peekram(self,adr,count):
        """Peeks data from CC2420 RAM."""
        data=[
            adr&0xFF,adr>>8,     # Address first.
            count&0xFF,count>>8  # Then length.
            ];
        self.writecmd(self.CCSPIAPP,0x84,len(data),data);
        return self.data;
    def pokeram(self,adr,data):
        """Pokes data into CC2420 RAM."""
        data=[adr&0xFF, adr>>8]+data;
        self.writecmd(self.CCSPIAPP,0x85,len(data),data);
        return;
    
    lastpacket: bytearray = bytearray(list(range(0,0xff)));

    def RF_txrxpacket(self,packet,timeout=1):
        data="\0";
        self.data=data;
        packet = [timeout&0xff, timeout>>8] + packet

        self.writecmd(self.CCSPIAPP,0x86,len(packet),packet);
        buffer=self.data;
        self.lastpacket=buffer;

        if(len(buffer)==0):
            return None;

        return buffer;

    def RF_rxpacket(self) -> Optional[bytes]:
        """Get a packet from the radio.  Returns None if none is
        waiting."""
        
        data: bytearray = bytearray([0]);
        self.data = data;
        buffer: bytearray = self.writecmd(self.CCSPIAPP,0x80,len(data),data);
        
        self.lastpacket: bytearray = buffer;

        if(len(buffer)==0):
            return None;

        return bytes(buffer);

    def RF_rxpacketrepeat(self):
        """Gets packets from the radio, ignoring all future requests so as
        not to waste time.  Call RF_rxpacket() after this."""
        
        self.writecmd(self.CCSPIAPP,0x91,0,None);
        return None;
    
    def RF_rxpacketdec(self):
        """Get and decrypt a packet from the radio.  Returns None if
        none is waiting."""
        
        data=bytearray([0]);
        self.data=data;
        self.writecmd(self.CCSPIAPP,0x90,len(data),data);
        buffer=self.data;
        
        self.lastpacket=buffer;
        if(len(buffer)==0):
            return None;
        
        return buffer;

    def RF_txpacket(self,packet):
        """Send a packet through the radio."""
        self.writecmd(self.CCSPIAPP,0x81,len(packet),packet);
        #time.sleep(1);
        #self.strobe(0x09);
        return;
    
    def RF_reflexjam(self,duration=0):
        """Place the device into reflexive jamming mode."""
        data = [duration&0xff,
                (duration>>8)&0xff];
        self.writecmd(self.CCSPIAPP,0xA0,len(data),data);
        return;

    def RF_reflexjam_autoack(self):
        """Place the device into reflexive jamming mode
           and that also sends a forged ACK if needed."""
        data = b""
        self.writecmd(self.CCSPIAPP,0xA1,len(data),data);
        print("Got:", data, "and", self.data)
        return;

    def RF_modulated_spectrum(self):
        """Hold a carrier wave on the present frequency."""
        # print("Don't know how to hold a carrier.")
        # 33.1 p.55:
        #  reset chip
        #  SXOSCON
        #  set MDMCTRL1.TX_MODE to 3   0x12  3:2 
        #  STXON                            0x04

        mdmctrl1=self.peek(0x12);
        mdmctrl1=mdmctrl1|0x00c0;  #MDMCTRL1.TX_MODE = 3
        self.poke(0x12, mdmctrl1); #MDMCTRL1

        mdmctrl1=self.peek(0x12);

        # http://e2e.ti.com/support/low_power_rf/f/155/t/15914.aspx?PageIndex=2
        #   suggests this
        self.strobe(0x02);         #STXCAL

        # is this necessary?
        self.strobe(0x09);         #SFLUSHTX

        self.strobe(0x04);         #STXON

    def RF_carrier(self):
        """Hold a carrier wave on the present frequency."""
        # print("Don't know how to hold a carrier.")
        # 33.1 p.54:
        #  reset chip
        #  SXOSCON
        #  set MDMCTRL1.TX_MODE to 2 or 3   0x12  3:2 
        #  set DACTST to 0x1800             0x2E
        #  STXON                            0x04

        mdmctrl1=self.peek(0x12);
        mdmctrl1=mdmctrl1|0x0080; 
        mdmctrl1=mdmctrl1&0x0080;  #MDMCTRL1.TX_MODE = 2
        self.poke(0x12, mdmctrl1); #MDMCTRL1

        mdmctrl1=self.peek(0x12);

        self.poke(0x2E, 0x1800);   #DACTST
        dactst=self.peek(0x2E);

        # see above for why this is here
        self.strobe(0x02);         #STXCAL
        self.strobe(0x09);         #SFLUSHTX

        self.strobe(0x04);         #STXON

    def RF_promiscuity(self,promiscuous=1):
        mdmctrl0=self.peek(0x11);
        if promiscuous>0:
            mdmctrl0=mdmctrl0&(~0x800);
        else:
            mdmctrl0=mdmctrl0|0x800;
        self.poke(0x11,mdmctrl0);
        return;
    def RF_autocrc(self,autocrc=1):
        mdmctrl0=self.peek(0x11);
        if autocrc==0:
            mdmctrl0=mdmctrl0&(~0x0020);
        else:
            mdmctrl0=mdmctrl0|0x0020;
        self.poke(0x11,mdmctrl0);
        return;
    def RF_autoack(self,autoack=1):
        mdmctrl0=self.peek(0x11);
        if autoack==0:
            mdmctrl0=mdmctrl0&(~0x0010);
        else:
            mdmctrl0=mdmctrl0|0x0010;
        self.poke(0x11,mdmctrl0);
        return;
    packetlen=16;
    def RF_setpacketlen(self,len=16):
        """Set the number of bytes in the expected payload."""
        #self.poke(0x11,len);
        self.packetlen=len;
    def RF_getpacketlen(self):
        """Set the number of bytes in the expected payload."""
        #len=self.peek(0x11);
        self.packetlen=len;
        return len;
    maclen=5;
    def RF_getmaclen(self):
        """Get the number of bytes in the MAC address."""
        choices=[0, 3, 4, 5];
        choice=self.peek(0x03)&3;
        self.maclen=choices[choice];
        return self.maclen;
    def RF_setmaclen(self,len):
        """Set the number of bytes in the MAC address."""
        choices=["illegal", "illegal", "illegal", 
                 1, 2, 3];
        choice=choices[len];
        self.poke(0x03,choice);
        self.maclen=len;
    def printpacket(self,packet,prefix="#"):
        print(self.packet2str(packet,prefix));
    def packet2str(self,packet,prefix="#"):
        s="";
        i=0;
        for foo in packet:
            s="%s %02x" % (s,ord(foo));
        return "%s%s" % (prefix,s);
        
    def printdissect(self,packet):
        try:
            from scapy.all import Dot15d4 # type: ignore
        except ImportError:
            print("To use packet disection, Scapy must be installed and have the Dot15d4 extension present.")
            print("try: hg clone http://hg.secdev.org/scapy-com");
            print("     sudo ./setup.py install");
        self.printpacket(packet);
        try:
            scapyd = Dot15d4(packet[1:]);
            scapyd.show();
        except:
            pass;

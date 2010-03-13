from killerbee import *
print "t0: get_dev_info then receive any packets available on channel 26"
kb = KillerBee()
print kb.get_dev_info()
kb.set_channel(26)
kb.sniffer_on()
print kb.pnext()
kb.sniffer_off()
kb.close()

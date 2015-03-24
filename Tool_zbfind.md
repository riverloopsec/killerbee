# Introduction #

zbfind provides a GTK-based GUI to the user which displays the results of a zbstumbler-like functionality. zbfind sends beacon requests as it cycles through channels and listens for a response, adding the response to a table as well as displaying signal strength on a gauge widget.


# Technical Details #

Reference RSSI is the RSSI measured in dBm at 1 meter distance from the transmitter. For the RZUSBSTICK, this is measured currently at -53 dBm, or an RSSI of 12.67. This corresponds to a maximum distance estimate of 10^((-58+91)/30) = 18.48m (61 feet). (The value was previously low, at -58 dBm. Thanks to Ben Ramsey for the updated research to improve the value! The update is in as of [r33](https://code.google.com/p/killerbee/source/detail?r=33).)

# Known Limitations #
  * Currently the RSSI adjustment/distance (refrssi) is only calculated for RZUSBSTICK devices. Other devices will incorrectly receive the same reference value.
import gps, os, time

session = gps.gps()
session.poll()
session.stream()

while 1:
    os.system('clear')
    session.poll()
    # a = altitude, d = date/time, m=mode,
    # o=postion/fix, s=status, y=satellites

    print
    print ' GPS reading'
    print '----------------------------------------'
    print 'fix         ' , ("NO_FIX","FIX","DGPS_FIX")[session.fix.mode - 1]
    print 'latitude    ' , session.fix.latitude
    print 'longitude   ' , session.fix.longitude
    print 'time utc    ' , session.utc, session.fix.time
    print 'altitude    ' , session.fix.altitude

    print
    print ' Satellites (total of', len(session.satellites) , ' in view)'
    for i in session.satellites:
        print '\t', i

    time.sleep(3)

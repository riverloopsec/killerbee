from .config import * # type: ignore
import MySQLdb # type: ignore

class DBReader:
    def __init__(self):
        self.conn = None
        self.db = None
        # Initalize the connection
        self.db = MySQLdb.connect(user=DB_USER, passwd=DB_PASS, db=DB_NAME, host=DB_HOST,
                                  port=DB_PORT)
        if self.db == None:
            raise Exception("DBLogger: Unable to connect to database.")
        self.conn = self.db.cursor()

    def close(self):
        if self.conn != None:
            self.conn.close()
            self.conn = None

    def query_one(self, table, columns, where):
        sql = "SELECT %s FROM %s WHERE %s LIMIT 1" % (columns, table, where)
        self.conn.execute(sql)
        if self.conn.rowcount == 1: return self.conn.fetchone()
        else: return None

    def query(self, sql):
        self.conn.execute(sql)
        if self.conn.rowcount >= 1:
            row = self.conn.fetchone()
            while row != None:
                yield row
                row = self.conn.fetchone()
        else: return

class DBLogger:
    def __init__(self, datasource=None, channel=None, page=0):
        self.conn = None

        if datasource == None: #datasource must be provided if DBLogger is desired
            return None

        self.db = None
        self.channel = channel
        self.page = page

        # Initalize the connection
        try:
            self.db = MySQLdb.connect(user=DB_USER, passwd=DB_PASS, db=DB_NAME, host=DB_HOST, port=DB_PORT)
        except Exception as e:
            raise Exception("DBLogger was unable to connect to the database: " \
                            +"(error %d): %s (Note: connection values should be in config.py)." \
                            +"error %s" % e)
        if self.db == None: #this backup check may be redundant
            raise Exception("DBLogger: Unable to connect to database.")
        self.conn = self.db.cursor()

        # Set the ds_id attribute to correspond to the requested data source name
        self.conn.execute("SELECT ds_id FROM datasources WHERE ds_name LIKE %s LIMIT 1", (datasource,))
        if self.conn.rowcount == 1: self.ds_id = self.conn.fetchone()
        else: print("No datasource found matching name:", datasource)

    def close(self):
        if self.conn != None:
            self.conn.close()
            self.conn = None

    def set_channel(self, chan, page):
        self.channel = chan
        self.page = page

    def add_packet(self, full=None, scapy=None,
                   bytes=None, rssi=None, location=None, datetime=None, channel=None, page=0):
        if (self.conn==None): raise Exception("DBLogger requires active connection status.")
        # Use values in 'full' parameter to provide data for undefined other parameters
        if bytes == None and 'bytes' in full: bytes = full['bytes']
        if rssi == None and 'rssi' in full: rssi = full['rssi']
        if datetime == None and 'datetime' in full: datetime = full['datetime']
        if location == None and 'location' in full: location = full['location']

        # Get the location ID, or create one, if GPS data is available
        loc_id = self.add_location(location) if location is not None else None

        # Dissect the packet's bytes, using the Scapy'd version in parameter scapy if provided
        if scapy == None:
            # Import Scapy extensions
            import logging # type: ignore
            logging.getLogger("scapy.runtime").setLevel(logging.ERROR)
            from scapy.all import Dot15d4 # type: ignore
            scapy = Dot15d4(bytes)
        #from kbutils import hexdump
        #scapy.show2()

        # This try/except logic is dumb, but Scapy will just throw an exception if the field doesn't exist
        try: srcaddr = scapy.src_addr
        except: srcaddr = None
        try: srcpan = scapy.src_panid
        except: srcpan = None
        srcdevid = self.add_device(srcaddr, srcpan)
        try: destaddr = scapy.dest_addr
        except: destaddr = None
        try: destpanid = scapy.dest_panid
        except: destpanid = None
        destdevid = self.add_device(destaddr, destpanid)

        sql = []
        sql.append("ds_id=%d" % self.ds_id)
        sql.append("db_datetime=NOW()")
        if datetime != None: sql.append("cap_datetime='%s'" % str(datetime))
        if self.channel != None: sql.append("channel=%d" % self.channel)
        if self.page: sql.append("page=%d" % self.page)
        if srcdevid != None: sql.append("source=%d" % srcdevid)
        if destdevid != None: sql.append("dest=%d" % destdevid)
        if rssi != None: sql.append("rssi=%d" % rssi)
        if loc_id != None: sql.append("loc_id=%d" % loc_id)
        if channel != None: sql.append("channel=%d" % channel) # TODO: bug? why is this in here twice?
        if page: sql.append("page=%d" % page) # TODO: bug? 
        sql.append("fcf_panidcompress=%d" % scapy.fcf_panidcompress)
        sql.append("fcf_ackreq=%d" % scapy.fcf_ackreq)
        sql.append("fcf_pending=%d" % scapy.fcf_pending)
        sql.append("fcf_security=%d" % scapy.fcf_security)
        sql.append("fcf_frametype=%d" % scapy.fcf_frametype)
        sql.append("fcf_srcaddrmode=%d" % scapy.fcf_srcaddrmode)
        sql.append("fcf_framever=%d" % scapy.fcf_framever)
        sql.append("fcf_destaddrmode=%d" % scapy.fcf_destaddrmode)
        sql.append("seqnum=%d" % scapy.seqnum)
        # adding the bytes of the packet are handled in the insert method b/c they are binary
        return self.insert(' '.join(['INSERT INTO packets SET', ', '.join(sql)]), packetbytes=bytes)

    def add_location(self, location):
        if (self.conn==None): raise Exception("DBLogger requires active connection status.")
        (lon, lat, alt) = location
        self.conn.execute("SELECT loc_id FROM locations WHERE %s AND %s AND %s LIMIT 1" %          \
                            ( ("longitude = '%f'" % lon) if lon != None else "longitude IS NULL" , \
                              ("latitude = '%f'" % lat) if lat != None else "latitude IS NULL"   , \
                              ("elevation = '%f'" % alt) if alt != None else "elevation IS NULL"  ))
        res = self.conn.fetchone()
        if (res != None):
            return res #location already in db, return loc_id
        else:
            self.conn.execute("INSERT INTO locations SET %s, %s, %s" %                            \
                            ( ("longitude = '%f'" % lon) if lon != None else "longitude = NULL" , \
                              ("latitude = '%f'" % lat) if lat != None else "latitude = NULL"   , \
                              ("elevation = '%f'" % alt) if alt != None else "elevation = NULL"  ))
            if self.conn.rowcount != 1: raise Exception("Location insert did not succeed.")
            self.db.commit()
            return self.conn.lastrowid

    def add_device(self, shortaddr, panid):
        if (self.conn==None): raise Exception("DBLogger requires active connection status.")
        self.conn.execute("SELECT dev_id FROM devices WHERE %s AND %s LIMIT 1" % \
                            ( ("short_addr = '%04x'" % shortaddr) if shortaddr != None else "short_addr IS NULL" , \
                              ("pan_id = '%04x'" % panid) if panid != None else "pan_id IS NULL" ))
        res = self.conn.fetchone()
        if (res != None):
            return res #device already exists
        else:
            self.conn.execute("INSERT INTO devices SET %s, %s" % \
                             (("short_addr = '%04x'" % shortaddr) if shortaddr != None else "short_addr = NULL" , \
                              ("pan_id = '%04x'" % panid) if panid != None else "pan_id = NULL" ))
            if self.conn.rowcount != 1: raise Exception("Device insert did not succeed.")
            self.db.commit()
            return self.conn.lastrowid

    def insert(self, sql, packetbytes=None):
        params = None
        if packetbytes != None:
            sql = sql + ", packetbytes=%s"
            params = (MySQLdb.Binary(packetbytes), )
        self.conn.execute(sql, params)
        if self.conn.rowcount != 1: raise Exception("DBLogger: Insert did not succeed.")
        self.db.commit()
        return self.conn.lastrowid

# Database (MySQL type assumed)
# This configuration is only required
#  if the dblog module is used.
# Database schema is expected to match that
#  created by killerbee/scripts/create_db.sql.
DB_HOST             = ""
DB_PORT             = 3306
DB_NAME             = ""
DB_USER             = ""
DB_PASS             = ""

# Device Support
# This configuration allow you to turn
#  support for devices on or off.
# If marked False, KillerBee will not try to
#  detect or load drivers for that device.
# Useful for disabling uncommon devices.
DEV_ENABLE_TELOSB = True
DEV_ENABLE_APIMOTE2 = True
DEV_ENABLE_APIMOTE1 = False
DEV_ENABLE_FREAKDUINO = False
DEV_ENABLE_SL_NODETEST = False
DEV_ENABLE_SL_BEEHIVE = False
DEV_ENABLE_ZIGDUINO = False

# Database (MySQL type assumed)
# This configuration is only required
#  if the dblog module is used.
# Database schema is expected to match that
#  created by killerbee/scripts/create_db.sql.
DB_HOST: str        = ""
DB_PORT: int        = 3306
DB_NAME: str        = ""
DB_USER: str        = ""
DB_PASS: str        = ""

# Device Support
# This configuration allow you to turn on
#  support for devices on or off.
# If marked False, KillerBee will not try to
#  detect or load drivers for that device.
# Useful for disabling uncommon devices.
DEV_ENABLE_FREAKDUINO: bool   = False
DEV_ENABLE_SL_NODETEST: bool  = False
DEV_ENABLE_SL_BEEHIVE: bool   = False
DEV_ENABLE_ZIGDUINO: bool     = False
DEV_ENABLE_TELOSB: bool       = False
DEV_ENABLE_APIMOTE2: bool     = True
DEV_ENABLE_APIMOTE1: bool     = False
DEV_ENABLE_BUMBLEBEE: bool    = False

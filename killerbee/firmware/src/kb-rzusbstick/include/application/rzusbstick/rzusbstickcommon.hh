/*! \file *********************************************************************
 *
 *      COPYRIGHT (c) ATMEL Norway, 1996-2006
 *
 *      The copyright to the document(s) herein is the property of
 *      ATMEL Norway, Norway.
 *
 *      The document(s) may be used and/or copied only with the written
 *      permission from ATMEL Norway or in accordance with the terms and
 *      conditions stipulated in the agreement/contract under which the
 *      document(s) have been supplied.
 *
 * \brief Communication Protocol Constants
 *
 *      Removed a lot of stuff here.....
 *
 *****************************************************************************/
#ifndef __RFUSBSTICK_COMMON_INCLUDED
#define __RFUSBSTICK_COMMON_INCLUDED

// General Primitives.
#define CMD_SIGN_OFF             0x00
#define CMD_SIGN_ON              0x01

#define CMD_GET_PARAMETER        0x02
#define CMD_SET_PARAMETER        0x03
#define CMD_SELF_TEST            0x04
#define CMD_CHECK_STACK_USAGE    0x05
#define CMD_MEMORY_TEST          0x06

#define CMD_SET_MODE             0x07

// Sniffer Primitives.
#define CMD_SET_CHANNEL          0x08
#define CMD_OPEN_STREAM          0x09
#define CMD_CLOSE_STREAM         0x0A
#define CMD_CHANNEL_SCAN         0x0B
#define CMD_CHANNEL_SCAN_STOP    0x0C
#define CMD_INJECT_FRAME         0x0D
#define CMD_JAMMER_ON            0x0E
#define CMD_JAMMER_OFF           0x0F

// TRX Primitives.
#define CMD_TRX_READ_REGISTER    0x10
#define CMD_TRX_WRITE_REGISTER   0x11
#define CMD_TRX_READ_FRAME       0x12
#define CMD_TRX_WRITE_FRAME      0x13
#define CMD_TRX_READ_SRAM        0x14
#define CMD_TRX_WRITE_SRAM       0x15
#define CMD_TRX_GET_PIN          0x16
#define CMD_TRX_SET_PIN          0x17


// Boot Loader Primitives.
#define CMD_ENTER_BOOT                0x18

// Stack Primitives.
#define CMD_MCPS_DATA_REQUEST         0x19   
#define CMD_MLME_ASSOCIATE_REQUEST    0x1A
#define CMD_MLME_DISASSOCIATE_REQUEST 0x1B
#define CMD_MLME_SCAN_REQUEST         0x1C
#define CMD_MLME_RX_ENABLE_REQUEST    0x1D
#define CMD_MLME_RX_DISABLE_REQUEST   0x1E
#define CMD_MLME_START_REQUEST        0x1F
#define CMD_MLME_RESET_REQUEST        0x20

// NWK Layer Primitives.
#define CMD_NLDE_DATA_REQUEST         (0x21)
#define CMD_NLME_FORMATION_REQUEST    (0x22)
#define CMD_NLME_PERMIT_JOIN_REQUEST  (0x23)
#define CMD_NLME_JOIN_REQUEST         (0x24)
#define CMD_NLME_LEAVE_REQUEST        (0x25)
#define CMD_NLME_DISCOVERY_REQUEST    (0x26)
#define CMD_NLME_START_ROUTER_REQUEST (0x27)

// Command range reserved for USB stick server (0x40-0x4F)
#define CMD_SERVER_PING               (0x40)
#define CMD_SERVER_ENUM_DEVICES       (0x41)
#define CMD_START_UPGRADE             (0x42)

// Bootloader commands (0xB0 - 0xBF)
#define CMD_BOOT_READ_SIGNATURE       (0xB0)
#define CMD_BOOT_GET_VERSION          (0xB1)
#define CMD_BOOT_START_APPLICATION    (0xB2)
#define CMD_BOOT_ERASE_FLASH          (0xB3)
#define CMD_BOOT_ERASE_EEPROM         (0xB4)
#define CMD_BOOT_PROG_FLASH           (0xB5)
#define CMD_BOOT_READ_FLASH           (0xB6)
#define CMD_BOOT_PROG_EEPROM          (0xB7)
#define CMD_BOOT_READ_EEPROM          (0xB8)
#define CMD_BOOT_READ_LOCKBITS        (0xB9)
#define CMD_BOOT_WRITE_LOCKBITS       (0xBA)
#define CMD_BOOT_READ_FUSES           (0xBB)

// Stream and Events.
#define EVENT_STREAM_AC_DATA          0x50
#define EVENT_SNIFFER_SCAN_COMPLETE   0x51
#define EVENT_SNIFFER_ERROR           0x52
#define EVENT_NWK_DATA                0x53
#define EVENT_NWK_JOIN                0x54
#define EVENT_NWK_LEAVE               0x55

// Event range reserved for USB stick server (0x70-0x7F)
#define EVENT_UPGRADE_STATUS          0x70

// Successful Responses.
#define RESP_SUCCESS             (0x80)
#define RESP_SYNTACTICAL_ERROR   (0x81)
#define RESP_SEMANTICAL_ERROR    (0x82)
#define RESP_HW_TIMEOUT          (0x83)
#define RESP_SIGN_ON             (0x84)
#define RESP_GET_PARAMETER       (0x85)
#define RESP_TRX_READ_REGISTER   (0x86)
#define RESP_TRX_READ_FRAME      (0x87)
#define RESP_TRX_READ_SRAM       (0x88)
#define RESP_TRX_GET_PIN         (0x89)
#define RESP_TRX_BUSY            (0x8A)
#define RESP_PRITMITIVE_FAILED   (0x8B)
#define RESP_PRITMITIVE_UNKNOWN  (0x8C)
#define RESP_COMMAND_UNKNOWN     (0x8D)
#define RESP_BUSY_SCANING        (0x8E)
#define RESP_BUSY_CAPTURING      (0x8F)
#define RESP_OUT_OF_MEMORY       (0x90)
#define RESP_BUSY_JAMMING        (0x91)
#define RESP_NOT_INITIALIZED     (0x92)
#define RESP_NOT_IMPLEMENTED     (0x93)
#define RESP_PRIMITIVE_FAILED    (0x94)
#define RESP_VRT_KERNEL_ERROR    (0x95) //!< Command could not be executed due to vrt_kernel error. 
#define RESP_BOOT_PARAM          (0x96)

// Response range reserved for USB stick server (0xF0-0xFF)
#define RESP_SERVER_ENUM_DEVICES (0xF0)

// General parameters
#define PARAM_SW_REV              0xC0
#define PARAM_IEEE_ADDRESS        0xC1

// Sign off parameters.
#define SHUT_DOWN (0x00)
#define RESET_ALL (0x01)
#define RESET_USB (0x02)
#define RESET_RF  (0x03)

// Parameters used by CMD_GET_PARAMETER.
#define PARAM_NWK_MAX_CHILDREN (0x00)
#define PARAM_NWK_MAX_ROUTRES  (0x01)
#define PARAM_NWK_MAX_DEPTH    (0x02)

// Set mode parameters
#define CMD_MODE_AC (0x00)
#define CMD_MODE_HAL (0x01)
#define CMD_MODE_MAC (0x02)
#define CMD_MODE_NWK (0x03)
#define CMD_MODE_NONE (0x04)

// Scan types
#define CMD_ED_SCAN      (0x00)
#define CMD_ACTIVE_SCAN  (0x01)
#define CMD_PASSIVE_SCAN (0x02)
#define CMD_ORPHAN_SCAN  (0x03)         

// Scan result variants
#define SCAN_VARIANT_ED           (0x00)
#define SCAN_VARIANT_PAN_SHORT    (0x01)
#define SCAN_VARIANT_PAN_LONG     (0x02)

// Pin definitions.
#define TRX_SLP_TR_PIN            (0x00)
#define TRX_RST_PIN               (0x01)
#define TRX_TST_PIN               (0x02)
#define TRX_NMBR_OF_PINS          (0x03)
#define TRX_MAX_PIN_VALUE         (0x01)

// Limits.
#define CMD_SIGN_OFF_LENGTH  (0x02)
#define CMD_SIGN_ON_LENGTH   (0x01)
#define CMD_SET_MODE_LENGTH   (0x02)

#define CMD_SET_CHANNEL_LENGTH (0x02)
#define CMD_OPEN_STREAM_LENGTH (0x01)
#define CMD_CLOSE_STREAM_LENGTH (0x01)
#define CMD_CHANNEL_SCAN_LENGTH (0x04)
#define CMD_JAMMER_OFF_LENGTH (0x01)

#define CMD_TRX_READ_REGISTER_LENGTH (0x02)
#define CMD_TRX_WRITE_REGISTER_LENGTH (0x03)
#define CMD_TRX_READ_PIN_LENGTH (0x02)
#define CMD_TRX_WRITE_PIN_LENGTH (0x03)

#define CMD_MAC_START_REQUEST_LENGTH (0x05)

#define NMBR_OF_SCAN_TYPES   (0x03)
#endif  // __RFUSBSTICK_COMMON_INCLUDED

// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  ..............
 *
 * \par Application note:
 *      AVR2017: RZRAVEN FW
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Id: usb_drv.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/


#ifndef _USB_DRV_H_
#define _USB_DRV_H_

//_____ I N C L U D E S ____________________________________________________
#include <stdint.h>
#include <stdbool.h>
#include "compiler.h"

typedef enum endpoint_parameter{ep_num, ep_type, ep_direction, ep_size, ep_bank, nyet_status} t_endpoint_parameter;

//_____ M A C R O S ________________________________________________________

#define EP_CONTROL            0
#define EP_1                  1
#define EP_2                  2
#define EP_3                  3
#define EP_4                  4
#define EP_5                  5
#define EP_6                  6
#define EP_7                  7

//! USB EndPoint
#define MSK_EP_DIR            0x7F
#define MSK_UADD              0x7F
#define MSK_EPTYPE            0xC0
#define MSK_EPSIZE            0x70
#define MSK_EPBK              0x0C
#define MSK_DTSEQ             0x0C
#define MSK_NBUSYBK           0x03
#define MSK_CURRBK            0x03
#define MSK_DAT               0xFF  // UEDATX
#define MSK_BYCTH             0x07  // UEBCHX
#define MSK_BYCTL             0xFF  // UEBCLX
#define MSK_EPINT             0x7F  // UEINT
#define MSK_HADDR             0xFF  // UHADDR

//! USB Pipe
#define MSK_PNUM              0x07  // UPNUM
#define MSK_PRST              0x7F  // UPRST
#define MSK_PTYPE             0xC0  // UPCFG0X
#define MSK_PTOKEN            0x30
#define MSK_PEPNUM            0x0F
#define MSK_PSIZE             0x70  // UPCFG1X
#define MSK_PBK               0x0C


//! Parameters for endpoint configuration
//! These define are the values used to enable and configure an endpoint.
#define TYPE_CONTROL             0
#define TYPE_ISOCHRONOUS         1
#define TYPE_BULK                2
#define TYPE_INTERRUPT           3
 //typedef enum ep_type {TYPE_CONTROL, TYPE_BULK, TYPE_ISOCHRONOUS, TYPE_INTERRUPT} e_ep_type;

#define DIRECTION_OUT            0
#define DIRECTION_IN             1
 //typedef enum ep_dir {DIRECTION_OUT, DIRECTION_IN} e_ep_dir;

#define SIZE_8                   0
#define SIZE_16                  1
#define SIZE_32                  2
#define SIZE_64                  3
#define SIZE_128                 4
#define SIZE_256                 5
#define SIZE_512                 6
#define SIZE_1024                7
 //typedef enum ep_size {SIZE_8,   SIZE_16,  SIZE_32,  SIZE_64,
 //                      SIZE_128, SIZE_256, SIZE_512, SIZE_1024} e_ep_size;

#define ONE_BANK                 0
#define TWO_BANKS                1
 //typedef enum ep_bank {ONE_BANK, TWO_BANKS} e_ep_bank;

#define NYET_ENABLED             0
#define NYET_DISABLED            1
 //typedef enum ep_nyet {NYET_DISABLED, NYET_ENABLED} e_ep_nyet;

#define Usb_build_ep_config0(type, dir, nyet)  ((type<<6) | (nyet<<1) | (dir))
#define Usb_build_ep_config1(size, bank     )  ((size<<4) | (bank<<2)        )
#define usb_configure_endpoint(num, type, dir, size, bank, nyet)             \
                                    ( Usb_select_endpoint(num),              \
                                      usb_config_ep(Usb_build_ep_config0(type, dir, nyet),\
                                                    Usb_build_ep_config1(size, bank)    ))

//! General USB management
//! These macros manage the USB controller.
#define Usb_enable_regulator()          (UHWCON |= (1<<UVREGE))
#define Usb_disable_regulator()         (UHWCON &= ~(1<<UVREGE))
#define Usb_enable_uid_pin()            (UHWCON |= (1<<UIDE))
#define Usb_disable_uid_pin()           (UHWCON &= ~(1<<UIDE))
#define Usb_force_device_mode()         (Usb_disable_uid_pin(), UHWCON |= (1<<UIMOD))
#define Usb_force_host_mode()           (Usb_disable_uid_pin(), UHWCON &= ~(1<<UIMOD))
#define Usb_enable_uvcon_pin()          (UHWCON |= (1<<UVCONE))
#define Usb_full_speed_mode()           (UHWCON |= (1<<UDSS))
#define Usb_low_speed_mode()            (UHWCON &= ~(1<<UDSS))




#define Usb_enable()                  (USBCON |= ((1<<USBE) | (1<<OTGPADE)))
#define Usb_disable()                 (USBCON &= ~((1<<USBE) | (1<<OTGPADE)))
//#define Usb_enable()                  (USBCON  |=  (1<<USBE))
//#define Usb_disable()                 (USBCON  &= ~(1<<USBE))
#define Usb_select_device()           (USBCON  &= ~(1<<HOST))
#define Usb_select_host()             (USBCON  |=  (1<<HOST))
#define Usb_freeze_clock()            (USBCON  |=  (1<<FRZCLK))
#define Usb_unfreeze_clock()          (USBCON  &= ~(1<<FRZCLK))
#define Usb_enable_id_interrupt()     (USBCON  |=  (1<<IDTE))
#define Usb_disable_id_interrupt()    (USBCON  &= ~(1<<IDTE))
#define Usb_enable_vbus_interrupt()   (USBCON  |=  (1<<VBUSTE))
#define Usb_disable_vbus_interrupt()  (USBCON  &= ~(1<<VBUSTE))
#define Is_usb_enabled()              ((USBCON  &   (1<<USBE))   ? true : false)
#define Is_usb_host_enabled()         ((USBCON  &   (1<<HOST))   ? true : false)
#define Is_usb_clock_freezed()        ((USBCON  &   (1<<FRZCLK)) ? true : false)

#define Is_usb_vbus_high()            ((USBSTA &   (1<<VBUS))    ? true : false)
#define Is_usb_high_speed()           ((USBSTA &   (1<<SPEED))   ? true : false)
#define Is_usb_id_device()            ((USBSTA &   (1<<ID))      ? true : false)

#define Usb_ack_id_transition()       (USBINT  = ~(1<<IDTI))
#define Usb_ack_vbus_transition()     (USBINT  = ~(1<<0))
#define Is_usb_id_transition()        ((USBINT &   (1<<IDTI))    ? true : false)
#define Is_usb_vbus_transition()      ((USBINT &   (1<<0))  ? true : false)

#define Usb_enter_dpram_mode()        (UDPADDH =  (1<<DPACC))
#define Usb_exit_dpram_mode()         (UDPADDH =  (uint8_t)~(1<<DPACC))
#define Usb_set_dpram_address(addr)   (UDPADDH =  (1<<DPACC) + ((Uint16)addr >> 8), UDPADDL = (Uchar)addr)
#define Usb_write_dpram_byte(val)     (UEDATX=val)
#define Usb_read_dpram_byte()			  (UEDATX)

#define Usb_enable_vbus()             (OTGCON  |=  (1<<VBUSREQ))
#define Usb_disable_vbus()            (OTGCON  &= ~(1<<VBUSREQ))
#define Usb_device_initiate_hnp()     (OTGCON  |=  (1<<HNPREQ))
#define Usb_host_accept_hnp()         (OTGCON  |=  (1<<HNPREQ))
#define Usb_host_reject_hnp()         (OTGCON  &= ~(1<<HNPREQ))
#define Usb_device_initiate_srp()     (OTGCON  |=  (1<<SRPREQ))
#define Usb_select_vbus_srp_method()  (OTGCON  |=  (1<<SRPSEL))
#define Usb_select_data_srp_method()  (OTGCON  &= ~(1<<SRPSEL))
#define Usb_enable_vbus_hw_control()  (OTGCON  &= ~(1<<VBUSHWC))
#define Usb_disable_vbus_hw_control() (OTGCON  |=  (1<<VBUSHWC))
#define Is_usb_vbus_enabled()         ((OTGCON &   (1<<VBUSREQ)) ? true : false)
#define Is_usb_hnp()                  ((OTGCON &   (1<<HNPREQ))  ? true : false)
#define Is_usb_device_srp()           ((OTGCON &   (1<<SRPREQ))  ? true : false)

#define Usb_enable_suspend_time_out_interrupt()   (OTGIEN  |=  (1<<STOE))
#define Usb_enable_hnp_error_interrupt()          (OTGIEN  |=  (1<<HNPERRE))
#define Usb_enable_role_exchange_interrupt()      (OTGIEN  |=  (1<<ROLEEXE))
#define Usb_enable_bconnection_error_interrupt()  (OTGIEN  |=  (1<<BCERRE))
#define Usb_enable_vbus_error_interrupt()         (OTGIEN  |=  (1<<VBERRE))
#define Usb_enable_srp_interrupt()                (OTGIEN  |=  (1<<SRPE))
#define Usb_disable_suspend_time_out_interrupt()  (OTGIEN  &= ~(1<<STOE))
#define Usb_disable_hnp_error_interrupt()         (OTGIEN  &= ~(1<<HNPERRE))
#define Usb_disable_role_exchange_interrupt()     (OTGIEN  &= ~(1<<ROLEEXE))
#define Usb_disable_bconnection_error_interrupt() (OTGIEN  &= ~(1<<BCERRE))
#define Usb_disable_vbus_error_interrupt()        (OTGIEN  &= ~(1<<VBERRE))
#define Usb_disable_srp_interrupt()               (OTGIEN  &= ~(1<<SRPE))

#define Usb_ack_suspend_time_out_interrupt()      (OTGIEN  &= ~(1<<STOI))
#define Usb_ack_hnp_error_interrupt()             (OTGIEN  &= ~(1<<HNPERRI))
#define Usb_ack_role_exchange_interrupt()         (OTGIEN  &= ~(1<<ROLEEXI))
#define Usb_ack_bconnection_error_interrupt()     (OTGIEN  &= ~(1<<BCERRI))
#define Usb_ack_vbus_error_interrupt()            (OTGIEN  &= ~(1<<VBERRI))
#define Usb_ack_srp_interrupt()                   (OTGIEN  &= ~(1<<SRPI))
#define Is_usb_suspend_time_out_interrupt()       ((OTGIEN &   (1<<STOI))    ? true : false)
#define Is_usb_hnp_error_interrupt()              ((OTGIEN &   (1<<HNPERRI)) ? true : false)
#define Is_usb_role_exchange_interrupt()          ((OTGIEN &   (1<<ROLEEXI)) ? true : false)
#define Is_usb_bconnection_error_interrupt()      ((OTGIEN &   (1<<BCERRI))  ? true : false)
#define Is_usb_vbus_error_interrupt()             ((OTGIEN &   (1<<VBERRI))  ? true : false)
#define Is_usb_srp_interrupt()                    ((OTGIEN &   (1<<SRPI))    ? true : false)


//! USB Device management
//! These macros manage the USB Device controller.
//!

#define Usb_initiate_remote_wake_up()             (UDCON   |=  (1<<RMWKUP))
#define Usb_detach()                              (UDCON   |=  (1<<DETACH))
#define Usb_attach()                              (UDCON   &= ~(1<<DETACH))
#define Is_usb_pending_remote_wake_up()           ((UDCON & (1<<RMWKUP)) ? true : false)
#define Is_usb_detached()                         ((UDCON & (1<<DETACH)) ? true : false)

#define Usb_ack_remote_wake_up_start()            (UDINT   = ~(1<<UPRSMI))
#define Usb_ack_resume()                          (UDINT   = ~(1<<EORSMI))
#define Usb_ack_wake_up()                         (UDINT   = ~(1<<WAKEUPI))
#define Usb_ack_reset()                           (UDINT   = ~(1<<EORSTI))
#define Usb_ack_sof()                             (UDINT   = ~(1<<SOFI))
#define Usb_ack_suspend()                         (UDINT   = ~(1<<SUSPI))
#define Is_usb_remote_wake_up_start()             ((UDINT &   (1<<UPRSMI))  ? true : false)
#define Is_usb_resume()                           ((UDINT &   (1<<EORSMI))  ? true : false)
#define Is_usb_wake_up()                          ((UDINT &   (1<<WAKEUPI)) ? true : false)
#define Is_usb_reset()                            ((UDINT &   (1<<EORSTI))  ? true : false)
#define Is_usb_sof()                              ((UDINT &   (1<<SOFI))    ? true : false)
#define Is_usb_suspend()                          ((UDINT &   (1<<SUSPI))   ? true : false)

#define Usb_enable_remote_wake_up_interrupt()     (UDIEN   |=  (1<<UPRSME))
#define Usb_enable_resume_interrupt()             (UDIEN   |=  (1<<EORSME))
#define Usb_enable_wake_up_interrupt()            (UDIEN   |=  (1<<WAKEUPE))
#define Usb_enable_reset_interrupt()              (UDIEN   |=  (1<<EORSTE))
#define Usb_enable_sof_interrupt()                (UDIEN   |=  (1<<SOFE))
#define Usb_enable_micro_sof_interrupt()          (UDIEN   |=  (1<<MSOFE))
#define Usb_enable_suspend_interrupt()            (UDIEN   |=  (1<<SUSPE))
#define Usb_disable_remote_wake_up_interrupt()    (UDIEN   &= ~(1<<UPRSME))
#define Usb_disable_resume_interrupt()            (UDIEN   &= ~(1<<EORSME))
#define Usb_disable_wake_up_interrupt()           (UDIEN   &= ~(1<<WAKEUPE))
#define Usb_disable_reset_interrupt()             (UDIEN   &= ~(1<<EORSTE))
#define Usb_disable_sof_interrupt()               (UDIEN   &= ~(1<<SOFE))
#define Usb_disable_micro_sof_interrupt()         (UDIEN   &= ~(1<<MSOFE))
#define Usb_disable_suspend_interrupt()           (UDIEN   &= ~(1<<SUSPE))

#define Usb_enable_address()                      (UDADDR  |=  (1<<ADDEN))
#define Usb_disable_address()                     (UDADDR  &= ~(1<<ADDEN))
#define Usb_configure_address(addr)               (UDADDR  =   (UDADDR & (1<<ADDEN)) | ((uint8_t)addr & MSK_UADD))

#define Usb_frame_number()                        ((uint16_t)((((uint16_t)UDFNUMH) << 8) | ((uint16_t)UDFNUML)))
#define Usb_micro_frame_number()                  ((uint8_t)(UDMFN & (1<<MFNUM))
#define Is_usb_frame_number_crc_error()           ((UDMFN & (1<<FNCERR)) ? true : false)



//! General endpoint management
//! These macros manage the common features of the endpoints
//!

#define Usb_select_endpoint(ep)                   (UENUM = (uint8_t)ep )

#define Usb_reset_endpoint(ep)                    (UERST   =   1 << (uint8_t)ep, UERST  =  0)

#define Usb_enable_endpoint()                     (UECONX  |=  (1<<EPEN))
#define Usb_enable_stall_handshake()              (UECONX  |=  (1<<STALLRQ))
#define Usb_reset_data_toggle()                   (UECONX  |=  (1<<RSTDT))
#define Usb_disable_endpoint()                    (UECONX  &= ~(1<<EPEN))
#define Usb_disable_stall_handshake()             (UECONX  |=  (1<<STALLRQC))
#define Usb_select_epnum_for_cpu()                (UECONX  &= ~(1<<EPNUMS))
#define Is_usb_endpoint_enabled()                 ((UECONX & (1<<EPEN))    ? true : false)
#define Is_usb_endpoint_stall_requested()         ((UECONX & (1<<STALLRQ)) ? true : false)

#define Usb_configure_endpoint_type(type)         (UECFG0X =   (UECFG0X & ~(1<<MSK_EPTYPE)) | ((uint8_t)type << 6))
#define Usb_enable_iso_autoswitch()               (UECFG0X |=  (1<<ISOSW))
#define Usb_enable_autoswitch()                   (UECFG0X |=  (1<<AUTOSW))
#define Usb_enable_nyet()                         (UECFG0X &= ~(1<<NYETDIS))
#define Usb_disable_iso_autoswitch()              (UECFG0X &= ~(1<<ISOSW))
#define Usb_disable_autoswitch()                  (UECFG0X &= ~(1<<AUTOSW))
#define Usb_disable_nyet()                        (UECFG0X |=  (1<<NYETDIS))
#define Usb_configure_endpoint_direction(dir)     (UECFG0X =   (UECFG0X & ~(1<<EPDIR))  | ((uint8_t)dir))

#define Usb_configure_endpoint_size(size)         (UECFG1X =   (UECFG1X & ~MSK_EPSIZE) | ((uint8_t)size << 4))
#define Usb_configure_endpoint_bank(bank)         (UECFG1X =   (UECFG1X & ~MSK_EPBK)   | ((uint8_t)bank << 2))
#define Usb_allocate_memory()                     (UECFG1X |=  (1<<ALLOC))
#define Usb_unallocate_memory()                   (UECFG1X &= ~(1<<ALLOC))

#define Usb_ack_overflow_interrupt()              (UESTA0X &= ~(1<<OVERFI))
#define Usb_ack_underflow_interrupt()             (UESTA0X &= ~(1<<UNDERFI))
#define Usb_ack_zlp()                             (UESTA0X &= ~(1<<ZLPSEEN))
#define Usb_data_toggle()                         ((UESTA0X&MSK_DTSEQ) >> 2)
#define Usb_nb_busy_bank()                        (UESTA0X &   MSK_NBUSYBK)
#define Is_usb_one_bank_busy()                    ((UESTA0X &  MSK_NBUSYBK) == 0 ? false : true)
#define Is_endpoint_configured()                  ((UESTA0X &  (1<<CFGOK))   ? true : false)
#define Is_usb_overflow()                         ((UESTA0X &  (1<<OVERFI))  ? true : false)
#define Is_usb_underflow()                        ((UESTA0X &  (1<<UNDERFI)) ? true : false)
#define Is_usb_zlp()                              ((UESTA0X &  (1<<ZLPSEEN)) ? true : false)

#define Usb_control_direction()                   ((UESTA1X &  (1<<CTRLDIR)) >> 2)
#define Usb_current_bank()                        ( UESTA1X & MSK_CURRBK)

#define Usb_ack_fifocon()                         (UEINTX &= ~(1<<FIFOCON))
#define Usb_ack_nak_in()                          (UEINTX &= ~(1<<NAKINI))
#define Usb_ack_nak_out()                         (UEINTX &= ~(1<<NAKOUTI))
#define Usb_ack_receive_setup()                   (UEINTX &= ~(1<<RXSTPI))
#define Usb_ack_receive_out()                     (UEINTX &= ~(1<<RXOUTI), Usb_ack_fifocon())
#define Usb_ack_stalled()                         (MSK_STALLEDI=   0)
#define Usb_ack_in_ready()                        (UEINTX &= ~(1<<TXINI), Usb_ack_fifocon())
#define Usb_kill_last_in_bank()                   (UENTTX |= (1<<RXOUTI)) 
#define Is_usb_read_enabled()                     (UEINTX&(1<<RWAL))
#define Is_usb_write_enabled()                    (UEINTX&(1<<RWAL))
#define Is_usb_read_control_enabled()             (UEINTX&(1<<TXINI))
#define Is_usb_receive_setup()                    (UEINTX&(1<<RXSTPI))
#define Is_usb_receive_out()                      (UEINTX&(1<<RXOUTI))
#define Is_usb_in_ready()                         (UEINTX&(1<<TXINI))
#define Usb_send_in()                             (UEINTX &= ~(1<<FIFOCON))
#define Usb_send_control_in()                     (UEINTX &= ~(1<<TXINI))
#define Usb_free_out_bank()                       (UEINTX &= ~(1<<FIFOCON))
#define Usb_ack_control_out()                     (UEINTX &= ~(1<<RXOUTI))

#define Usb_enable_flow_error_interrupt()         (UEIENX  |=  (1<<FLERRE))
#define Usb_enable_nak_in_interrupt()             (UEIENX  |=  (1<<NAKINE))
#define Usb_enable_nak_out_interrupt()            (UEIENX  |=  (1<<NAKOUTE))
#define Usb_enable_receive_setup_interrupt()      (UEIENX  |=  (1<<RXSTPE))
#define Usb_enable_receive_out_interrupt()        (UEIENX  |=  (1<<RXOUTE))
#define Usb_enable_stalled_interrupt()            (UEIENX  |=  (1<<STALLEDE))
#define Usb_enable_in_ready_interrupt()           (UEIENX  |=  (1<<TXIN))
#define Usb_disable_flow_error_interrupt()        (UEIENX  &= ~(1<<FLERRE))
#define Usb_disable_nak_in_interrupt()            (UEIENX  &= ~(1<<NAKINE))
#define Usb_disable_nak_out_interrupt()           (UEIENX  &= ~(1<<NAKOUTE))
#define Usb_disable_receive_setup_interrupt()     (UEIENX  &= ~(1<<RXSTPE))
#define Usb_disable_receive_out_interrupt()       (UEIENX  &= ~(1<<RXOUTE))
#define Usb_disable_stalled_interrupt()           (UEIENX  &= ~(1<<STALLEDE))
#define Usb_disable_in_ready_interrupt()          (UEIENX  &= ~(1<<TXIN))

#define Usb_read_byte()                           (UEDATX)
#define Usb_write_byte(byte)                      (UEDATX  =   (uint8_t)byte)

#define Usb_byte_counter()                        ((((uint16_t)UEBCHX) << 8) | (UEBCLX))
#define Usb_byte_counter_8()                      ((uint8_t)UEBCLX)

#define Usb_interrupt_flags()                     (UEINT != 0x00)
#define Is_usb_endpoint_event()                   (Usb_interrupt_flags())

// ADVANCED MACROS
#define Usb_select_ep_for_cpu(ep)                 (Usb_select_epnum_for_cpu(), Usb_select_endpoint(ep))




//! wSWAP
//! This macro swaps the uint8_t order in words.
//!
//! @param x        (uint16_t) the 16 bit word to swap
//!
//! @return         (uint16_t) the 16 bit word x with the 2 bytes swaped

#define wSWAP(x)        \
   (   (((x)>>8)&0x00FF) \
   |   (((x)<<8)&0xFF00) \
   )


//! Usb_write_word_enum_struc
//! This macro help to fill the uint16_t fill in USB enumeration struct.
//! Depending on the CPU architecture, the macro swap or not the nibbles
//!
//! @param x        (uint16_t) the 16 bit word to be written
//!
//! @return         (uint16_t) the 16 bit word written
#if !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)
	#error YOU MUST Define the Endian Type of target: LITTLE_ENDIAN or BIG_ENDIAN
#endif
#ifdef LITTLE_ENDIAN
	#define Usb_write_word_enum_struc(x)	(x)
#else //BIG_ENDIAN
	#define Usb_write_word_enum_struc(x)	(wSWAP(x))
#endif


//_____ D E C L A R A T I O N ______________________________________________

uint8_t      usb_config_ep                (uint8_t, uint8_t);
uint8_t      usb_select_enpoint_interrupt (void);
uint16_t     usb_get_nb_byte_epw          (void);
uint8_t      usb_send_packet              (uint8_t , uint8_t*, uint8_t);
uint8_t      usb_read_packet              (uint8_t , uint8_t*, uint8_t);
void    usb_halt_endpoint            (uint8_t);
void    usb_reset_endpoint           (uint8_t);
uint8_t      usb_init_device              (void);



#endif  // _USB_DRV_H_


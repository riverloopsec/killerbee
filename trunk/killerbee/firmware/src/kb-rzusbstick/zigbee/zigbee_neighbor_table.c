// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the Neighbor Table functionality needed for the
 *         ZigBee NWK layer
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
 * $Id: zigbee_neighbor_table.c 41241 2008-05-03 17:07:52Z vkbakken $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 
/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"

#include "zigbee_conf.h"
#include "zigbee_const.h"
#include "zigbee_neighbor_table.h"
#include "zigbee_nib.h"
/*================================= MACROS           =========================*/
/*================================= TYEPDEFS         =========================*/
/*================================= GLOBAL VARIABLES =========================*/
/*! \brief Pointer to the last element in the Neighbor Table. */
zigbee_neighbor_table_item_t *last;


/*! \brief Pointer to the first element in the Neighbor Table. */
zigbee_neighbor_table_item_t *first;


uint8_t neighbors; //!< Current numner of items in the Neighbor Table.
/*================================= LOCAL VARIABLES  =========================*/
static zigbee_neighbor_table_item_t neighbor_table[ZIGBEE_MAX_NMBR_OF_CHILDREN];
static zigbee_neighbor_table_item_t *free_list;    //!< Pointer to the next free zigbee_neighbor_table_item_t.
static uint8_t items_in_free_list;

static uint8_t routers_joined; //!< Number of routers the node has joined.
static uint8_t devices_joined; //!< Total number of end devices the node has joined.
/*================================= PROTOTYPES       =========================*/

void zigbee_neighbor_table_init(void) {
    /* Initialize the linked list. */
    neighbors = 0;
    
    last = NULL;
    first = NULL;
    
    /* Initialize the list of free zigbee_neighbor_table_item_t's */
    free_list = neighbor_table;
    zigbee_neighbor_table_item_t **link = (zigbee_neighbor_table_item_t **)neighbor_table;
    zigbee_neighbor_table_item_t *next_item  = (zigbee_neighbor_table_item_t *)((uint16_t)neighbor_table + sizeof(zigbee_neighbor_table_item_t));
    
    /* Set-up links between the free items in the queue. */
    for (uint8_t i = 0; i < (ZIGBEE_MAX_NMBR_OF_CHILDREN - 1); i++) {
        *link = (zigbee_neighbor_table_item_t *)next_item;
        link = (zigbee_neighbor_table_item_t **)next_item;
        next_item++;
    }
    
    /* Set the last link to NULL and the number of free items.  */
    *link = NULL;
    items_in_free_list = ZIGBEE_MAX_NMBR_OF_CHILDREN;
    
    routers_joined = 0;
    devices_joined = 0;
}


zigbee_neighbor_table_item_t* zigbee_neighbor_table_get(void) {
    /* Check if there is any packets left to allocate. */
    ENTER_CRITICAL_REGION();

    if (0 == items_in_free_list) {
        LEAVE_CRITICAL_REGION();
        return (zigbee_neighbor_table_item_t*)NULL;
    }
    
    /* Get hold of the first free packet available from the queue. */
    zigbee_neighbor_table_item_t *allocated = free_list;
    free_list = *(zigbee_neighbor_table_item_t **)allocated;
    items_in_free_list--;
    
    LEAVE_CRITICAL_REGION();
    
    return allocated;
}


void zigbee_neighbor_table_put(zigbee_neighbor_table_item_t *nb_item) {
    /* Check if all memory already has been freed, and that it is not NULL. */
    ENTER_CRITICAL_REGION();
    if (ZIGBEE_MAX_NMBR_OF_CHILDREN == items_in_free_list) {
        LEAVE_CRITICAL_REGION();
        return;
    }
    
    if (((zigbee_neighbor_table_item_t *)NULL) == nb_item) {
        LEAVE_CRITICAL_REGION();
        return;
    }
    
    /* There is room for this packet. Added it to the list of free packets. */
    *(zigbee_neighbor_table_item_t **)nb_item = free_list;
    free_list = nb_item;
    items_in_free_list++;
    LEAVE_CRITICAL_REGION();
}


void zigbee_neighbor_table_append(zigbee_neighbor_table_item_t *nb_item) {
    if (NULL == nb_item) { return; }
    
    /* Insert at the end of the neighbor table. */
    ENTER_CRITICAL_REGION();
    
    if (NULL == last) {
        last = nb_item;
        first = nb_item;
        
        nb_item->prev = NULL;
    } else {
        last->next = nb_item;
        nb_item->prev = last;
        last = nb_item;
    }
    
    nb_item->next = NULL;
    
    LEAVE_CRITICAL_REGION();
    neighbors++;
}


void zigbee_neighbor_table_delete(zigbee_neighbor_table_item_t *nb_item) {
    if (NULL == nb_item) {
        return;
    }
    
    ENTER_CRITICAL_REGION();
    
    if (ZIGBEE_TYPE_ROUTER == (nb_item->DeviceType)) {
        routers_joined--;          
    } else {
        devices_joined--;      
    }
    
    if (NULL == (nb_item->prev)) {
        first = nb_item->next;
    } else {
        nb_item->prev->next = nb_item->next;    
    }
    
    if (NULL == (nb_item->next)) {
        last = nb_item->prev;
    } else {
        nb_item->next->prev = nb_item->prev;
    }
    
    LEAVE_CRITICAL_REGION();
    
    neighbors--;
    
    /* Free the item as well into the list of free Neighbor Table Items. */
    zigbee_neighbor_table_put(nb_item);
}


zigbee_neighbor_table_item_t *zigbee_neighbor_table_find(uint16_t short_address) {
    if (0 == neighbors) {
        return (zigbee_neighbor_table_item_t *)NULL;
    }
    
    zigbee_neighbor_table_item_t *link = first;
    
    zigbee_neighbor_table_item_t *scan_result = NULL;
    for(uint8_t i = 0; i < neighbors; i++) {
        if (short_address == (link->NetworkAddress)) {
            scan_result = link;
            break;
        }
        
        link = link->next;
    }
    
    return scan_result;
}


zigbee_neighbor_table_item_t *zigbee_neighbor_table_find_long(uint64_t long_address) {
    if (0 == neighbors) {
        return (zigbee_neighbor_table_item_t *)NULL;
    }
    
    zigbee_neighbor_table_item_t *link = first;
    
    zigbee_neighbor_table_item_t *scan_result = (zigbee_neighbor_table_item_t *)NULL;
    for(uint8_t i = 0; i < neighbors; i++) {
        if (long_address == (link->ExtendedAddress)) {
            scan_result = link;
            break;
        }
        
        link = link->next;
    }
    
    return scan_result;
}


zigbee_neighbor_table_item_t *zigbee_neighbor_table_add_router(void) {
    if (ZIGBEE_MAX_NMBR_OF_CHILDREN <= (devices_joined + routers_joined)) {
        return (zigbee_neighbor_table_item_t *)NULL;
    }
    
    if (ZIGBEE_MAX_NMBR_OF_ROUTERS <= routers_joined) {
        return (zigbee_neighbor_table_item_t *)NULL;
    }
    
    zigbee_neighbor_table_item_t *router = zigbee_neighbor_table_get();
    
    if (NULL == router) {
        return (zigbee_neighbor_table_item_t *)NULL;
    }
    
    /* Allocate short address. */
    uint16_t router_address = 1 + NWK_NIB_GET_NWK_ADDRESS();
    
    /* Look for potential holes in the tree addressing scheme. Ex. a node that  */
    for (uint8_t i = 0; i < routers_joined; i++) {
        zigbee_neighbor_table_item_t *this_device = zigbee_neighbor_table_find(router_address);
            
        if (NULL == this_device) {
            break;
        }
        
        router_address += NWK_NIB_GET_ADDRESS_INCREMENT();
    }
    
    /* Set address and router information. */
    router->NetworkAddress = router_address;
    router->DeviceType = ZIGBEE_TYPE_ROUTER;
        
    /* Update number of routers joined. */
    routers_joined++;
        
    /* Add the router to the Neighbor Table. */
    zigbee_neighbor_table_append(router);
    
    return router;
}


zigbee_neighbor_table_item_t *zigbee_neighbor_table_add_device(void) {
    if (ZIGBEE_MAX_NMBR_OF_CHILDREN <= (devices_joined + routers_joined)) {
        return (zigbee_neighbor_table_item_t *)NULL;
    }
    
    zigbee_neighbor_table_item_t *device = zigbee_neighbor_table_get();
    
    if (NULL == device) {
        return device;
    }
    
    /* Allocate short address. */
    uint16_t device_address = 1 + NWK_NIB_GET_NWK_ADDRESS() + \
                              (NWK_NIB_GET_ADDRESS_INCREMENT() * ZIGBEE_MAX_NMBR_OF_ROUTERS);
        
    /* Look for potential holes in the tree addressing scheme. Ex. a node that  */
    for (uint8_t i = 0; i < devices_joined; i++) {
        zigbee_neighbor_table_item_t *this_device = zigbee_neighbor_table_find(device_address);
            
        if (NULL == this_device) {
            break;
        }
        
        device_address++;
    }
    
    device->NetworkAddress = device_address;
    device->DeviceType = ZIGBEE_TYPE_DEVICE;
        
    /* Update number of devices joined. */
    devices_joined++;
        
    /* Add the router to the Neighbor Table. */
    zigbee_neighbor_table_append(device);
    
    return device;
}


uint8_t zigbee_neighbor_table_find_depth(uint16_t parent_address, uint16_t node_address) {
#if (ZIGBEE_MAX_NMBR_OF_ROUTERS == 0)
    return 1;
#else
    /* Check if the parent is the network coordinator. */
    if (0x0000 == parent_address) {
        return 1;
    }

    for (uint8_t d = 1; d < ZIGBEE_MAX_NWK_DEPTH; d++) {
        uint16_t c_skip = zigbee_nib_c_skip(d);
        uint16_t router_address = 1;
        for (uint8_t r = 0; r < ZIGBEE_MAX_NMBR_OF_ROUTERS; r++) {
            if (parent_address == router_address) {
                return (d + 1);
            }
            
            router_address +=  c_skip;
        }
    }
    
    return 0;
#endif
}
/*EOF*/

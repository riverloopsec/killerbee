// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  This file implements the API for accessing the Neighbor Table
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
 * $Id: zigbee_neighbor_table.h 41144 2008-04-29 12:42:28Z ihanssen $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
 #ifndef ZIGBEE_NEIGHBOR_TABLE_H
#define ZIGBEE_NEIGHBOR_TABLE_H

/*================================= INCLUDES         =========================*/
#include <stdint.h>
#include <stdbool.h>

#include "zigbee_internal_api.h"
#include "zigbee_const.h"
#include "zigbee_msg_types.h"
/*================================= MACROS           =========================*/
/*! \brief This function-like macro will return the first item in the Neighbor Table. */
#define NEIGHBOR_TABLE_GET_FIRST() (first)


/*! \brief This function-like macro will return the next item in the Neighbor Table. */
#define NEIGHBOR_TABLE_GET_NEXT(item) (item->next)


/*! \brief Returns true if current item is the last in the Neighbor Table. */
#define NEIGHBOR_TABLE_IS_LAST_ELEMENT(item) ((NULL == (item)) ? true : false)


/*! \brief This fucntion-like macro returns current number of elements in the Neighbor Table. */
#define NEIGHBOR_TABLE_GET_SIZE() (neighbors)
/*================================= TYEPDEFS         =========================*/
/** Element of the neighbor table. */
typedef struct NEIGHBOR_TABLE_ITEM_STRUCT_TAG {
    struct NEIGHBOR_TABLE_ITEM_STRUCT_TAG *next;
    struct NEIGHBOR_TABLE_ITEM_STRUCT_TAG *prev;
    /** The network descriptor part of this element. */
    network_desc_t ndesc;
    /** The 64-bit extended address of the neighbor. */
    uint64_t ExtendedAddress;
    /** The 16-bit address of the neighbor. */
    uint16_t NetworkAddress;
    /** The type of the neighbor. */
    zigbee_device_t DeviceType;
    /** Indicates the neighbor's ability to leave the Rx enabled. */
    uint8_t RxOnWhenIdle;
    /** The relationship of the neighbor to us. */
    zigbee_relation_t Relationship;
    /** The depth of our neighbor. */
    uint8_t Depth;
    /** Number of unsuccessful transmission attempts. */
    uint8_t TransmitFailure;
    /** This neighbor is a potential parent. */
    uint8_t PotentialParent;
    /** Estimated link quality for transmissions from this device. */
    uint8_t LQI;
} zigbee_neighbor_table_item_t;
/*================================= GLOBAL VARIABLES =========================*/
/* Do not access these variable directly. They are to be used in the function-like
 * macros defined above.
 */
extern zigbee_neighbor_table_item_t *last;
extern zigbee_neighbor_table_item_t *first;;
extern uint8_t neighbors;
/*================================= LOCAL VARIABLES  =========================*/
/*================================= PROTOTYPES       =========================*/

/*! \brief This function will initialize the neighbor table. */
void zigbee_neighbor_table_init(void);

/*! \brief This function will return a pointer to a free neighbor table item
 *         if any available.
 *
 *  \returns Pointer to zigbee_neighbor_table_item_t if any available, NULL else.
 *
 */
zigbee_neighbor_table_item_t* zigbee_neighbor_table_get(void);

/*! \brief This function is used to return a perviously allocated 
 *         zigbee_neighbor_table_item_t item.
 *
 *  \param[in] nb_item Pointer to item that is freed/returned.
 *
 */
void zigbee_neighbor_table_put(zigbee_neighbor_table_item_t *nb_item);

/*! \brief Add an item to the neighbor table.
 *
 *  \param[in] nb_item Pointer to item to add.
 *
 */
void zigbee_neighbor_table_append(zigbee_neighbor_table_item_t *nb_item);

/*! \brief This function will delete a certain item from the Neighbor table, and
 *         also free the associtaed memory.
 *
 *  \param[in] nb_item Pointer to the Neighbor Table Item that is to be deleted.
 *
 */
void zigbee_neighbor_table_delete(zigbee_neighbor_table_item_t *nb_item);

/*! \brief This function will search the neighbor table for items with this address
 *         and return a pointer to the first occurence.
 *
 *  \param[in] short_address Short address of the node we are searching for.
 *  \returns Pointer to item is found. Null is returned in any other case.
 *
 */
zigbee_neighbor_table_item_t *zigbee_neighbor_table_find(uint16_t short_address);

/*! \brief This function will search the neighbor table for items with this IEEE
 *         address and return a pointer to the first occurence.
 *
 *  \param[in] long_address IEEE address of the node we are searching for.
 *  \returns Pointer to item is found. Null is returned in any other case.
 */
zigbee_neighbor_table_item_t *zigbee_neighbor_table_find_long(uint64_t long_address);

/*! \brief This function will try to add a child router to this device's Neighbor
 *         Table. A short address space will be allocated.
 *
 *  \returns A pointer to the router's item in the Neighbor Table will be returned.
 *           NULL is returned if the router could not be added.
 *
 */
zigbee_neighbor_table_item_t *zigbee_neighbor_table_add_router(void);

/*! \brief This function will try to add a child router to this device's Neighbor
 *         Table. A short address will be allocated.
 *
 *  \returns A pointer to the device's item in the Neighbor Table will be returned.
 *           NULL is returned if the device could not be added.
 *
 */
zigbee_neighbor_table_item_t *zigbee_neighbor_table_add_device(void);

/*! \brief This function is used by a router to calculate it's depth in the network.
 *
 *  \param[in] parent_address Short address of the node's parent.
 *  \param[in] node_address Short address allocated to this node through the 
 *                          association procedure.
 *
 *  \returns The node's depth in the network will be returned. A depth that equals 
 *           zero indicates an error.
 *
 */
uint8_t zigbee_neighbor_table_find_depth(uint16_t parent_address, uint16_t node_address);
#endif
/*EOF*/

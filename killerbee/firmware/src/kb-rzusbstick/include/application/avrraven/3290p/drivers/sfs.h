// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  API for the simple file system
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
 * $Id: fs.h 41490 2008-05-09 13:26:35Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/
#ifndef __SFS_H__
#define __SFS_H__

#include <stdint.h>

#include "sfs_conf.h"

// Error codes
typedef enum {
    SFS_LIST_START,
    SFS_LIST_CONTINUE,
    SFS_LIST_HEADER
}sfs_list_option_t;

/* Open modes */
#define SFS_OPEN_MODE_RD     0x01
#define SFS_OPEN_MODE_WR     0x02
#define SFS_OPEN_MODE_RDWR   0x03
#define SFS_OPEN_MODE_APPEND 0x04
#define SFS_OPEN_MODE_TRUNC  0x08
#define SFS_OPEN_MODE_CREATE 0x10
#define SFS_OPEN_MODE_BINARY 0x20

#define SFS_FILE_SEGMENT_HEADER_SIZE     (16)

#define SFS_FILE_SEGMENT_DATA_SIZE       (SFS_BLOCK_SIZE - SFS_FILE_SEGMENT_HEADER_SIZE)

#define SFS_LIST_LINE_SIZE               (50)

#define SFS_FILE_FLAG_COUNT              (8)

#define SFS_MAX_BASENAME_SIZE            (8)
#define SFS_MAX_EXTENSION_SIZE           (3)
#define SFS_MAX_FILENAME_SIZE            (SFS_MAX_BASENAME_SIZE + SFS_MAX_EXTENSION_SIZE)

#define SFS_MAX_BASENAME_BUFFER_SIZE     (SFS_MAX_BASENAME_SIZE + 1)
#define SFS_MAX_EXTENSION_BUFFER_SIZE    (SFS_MAX_EXTENSION_SIZE + 1)
#define SFS_MAX_FILENAME_BUFFER_SIZE     (SFS_MAX_FILENAME_SIZE + 1 + 1) // The extra "+1" is for the '.'

#define SFS_SEG_HDR_SIZE                 (sizeof(sfs_fseg_t))
#define SFS_SEG_HDR_OFFSET               (0)
#define SFS_SEG_HDR_NEXT_SIZE            (sizeof(sfs_fseg_next_t))
#define SFS_SEG_HDR_NEXT_OFFSET          (0)
#define SFS_SEG_HDR_PREV_SIZE            (sizeof(sfs_fseg_prev_t))
#define SFS_SEG_HDR_PREV_OFFSET          (sizeof(sfs_fseg_next_t))

#define SFS_ENTRY_NEXT_OFFSET            (SFS_BASENAME_BUFFER_SIZE+SFS_EXTENSION_BUFFER_SIZE+sizeof(sfs_faddr_t)+sizeof(sfs_fsize_t)+sizeof(sfs_fflags_t))

#define SFS_FILE_SEGMENT_SIZE            (SFS_BLOCK_SIZE - SFS_FILE_SEGMENT_HEADER_SIZE)


#define SFS_SYS_START                    (0x00000000)
#define SFS_FREE_SEGMENT_HEAD_PTR_ADR    (SFS_SYS_START)
#define SFS_FREE_SEGMENT_HEAD_PTR_SIZE   (sizeof(sfs_faddr_t))

#define SFS_FREE_SEGMENT_TAIL_ADR        (SFS_FREE_SEGMENT_HEAD_PTR_ADR+SFS_FREE_SEGMENT_HEAD_PTR_SIZE)
#define SFS_FREE_SEGMENT_TAIL_SIZE       (sizeof(sfs_faddr_t))


#define SFS_ENTRY_CNT                    SFS_MAX_FILES
#define SFS_ENTRY_SIZE                   (sizeof(sfs_file_t))
#define SFS_ENTRY_TBL_START              (SFS_ENTRY_SIZE)
#define SFS_ENTRY_TBL_SIZE               (SFS_ENTRY_CNT * SFS_ENTRY_SIZE)
#define SFS_ENTRY_TBL_END                (SFS_ENTRY_TBL_START + SFS_ENTRY_TBL_SIZE)

#define SFS_ENTRY_FILENAME_OFFSET        (0)
#define SFS_ENTRY_FILENAME_BUFFER_SIZE   SFS_MAX_FILENAME_SIZE

#define SFS_ENTRY_BASENAME_OFFSET        (0)
#define SFS_ENTRY_BASENAME_BUFFER_SIZE   SFS_MAX_BASENAME_SIZE

#define SFS_ENTRY_EXTENSION_OFFSET       SFS_MAX_BASENAME_SIZE
#define SFS_ENTRY_EXTENSION_BUFFER_SIZE  SFS_MAX_EXTENSION_SIZE

#define SFS_ENTRY_SIZE_OFFSET            SFS_MAX_FILENAME_SIZE
#define SFS_ENTRY_SIZE_SIZE              (sizeof(sfs_fsize_t))


#define SFS_FILE_SECT_START              ((uint32_t)SFS_BLOCK_SIZE*8UL)
#define SFS_FILE_SECT_END                ((uint32_t)SFS_BLOCK_SIZE*(uint32_t)SFS_BLOCK_COUNT)
#define SFS_FILE_SECT_LAST               ((uint32_t)SFS_FILE_SECT_END-(uint32_t)SFS_BLOCK_SIZE)

typedef int8_t      sfs_fhandle_t;
typedef uint32_t    sfs_faddr_t;
typedef uint32_t    sfs_fsize_t;
typedef uint32_t    sfs_findex_t;
typedef uint16_t    sfs_sindex_t;
typedef uint8_t     sfs_fmode_t;
typedef uint8_t     sfs_fflags_t;
typedef unsigned char sfs_fname_t;
typedef unsigned char sfs_ftype_t;
typedef uint8_t     sfs_ret_t;

typedef sfs_faddr_t sfs_fseg_next_t;
typedef sfs_faddr_t sfs_fseg_prev_t;

typedef struct sfs_fseg{
    sfs_fseg_next_t next;
    sfs_fseg_prev_t prev;
} sfs_fseg_t;

typedef enum {
    SFS_FGET_FWD,
    SFS_FGET_REV,
    SFS_FGET_RESTART,
} sfs_fget_t;

typedef struct sfs_stream{
    sfs_findex_t    wr_ptr;
    sfs_findex_t    rd_ptr;
    sfs_faddr_t     entry;
    sfs_fmode_t     mode;
    sfs_fhandle_t   handle;
    sfs_fsize_t     size;
    int             rd_seg;
    int             wr_seg;
}sfs_fstream_t;


// ok with small buffer since SFS_FGET_RESTART makes sfs_fget() return withot using it
#define sfs_fget_restart()\
    do {\
        unsigned char file_types[6];\
        unsigned char file_name[1];\
        strncpy_P((char*)file_types, "*.*", sizeof(file_types)+1);\
        sfs_fget(file_types, (unsigned char*)file_name, SFS_FGET_RESTART);\
    } while(0);


#ifdef SFS_USE_ERROR_STRINGS
typedef PROGMEM_STRING_T sfs_err_t;
#endif


int sfs_init(void);
void sfs_deinit(void);

/*! \brief          Create file
 *
 *
 * \param[in]		filename			Name of file to create
 *
 * \return                              0 if OK, EOF if error
 */
int sfs_fcreate(const unsigned char* filename);

/*! \brief          Remove file
 *
 *
 * \param[in]		filename			Name of file to remove
 *
 * \return                              0 if OK, EOF if error
 */
int sfs_remove(const unsigned char* filename);


/*! \brief          Open file
 *
 *
 * \param[in]		filename			Name of file to open
 * \param[in]		mode			    Open mode (not ANSI C)
 *
 * \return                              Pointer to the opened file stream (NULL if error)
 */
sfs_fstream_t* sfs_fopen(const unsigned char* filename, uint8_t mode);


/*! \brief          Close file
 *
 * \param[in]		stream			    File stream pointer
 *
 * \return                              EOF on error
 */
int sfs_fclose(sfs_fstream_t* stream);


/*! \brief          Character put
 *
 * \param[in]		c			        Character to write to file
 * \param[in]		stream			    File stream pointer
 *
 * \return                              Number if bytes written
 */
int sfs_fputc(unsigned char c, sfs_fstream_t* stream);


/*! \brief          Character get
 *
 * \param[in]		stream			    File stream pointer
 *
 * \return                              Read charcter. EOF on error
 */
int sfs_fgetc(sfs_fstream_t* stream);


/*! \brief          Flush file
 *
 *                  Read- and write pointers are reset
 *
 * \param[in]		stream			    File stream pointer
 *
 * \return                              EOF on error
 */
int sfs_fflush(sfs_fstream_t* stream);


/*! \brief          Read data from file
 *
 * \param[out]  data			    Buffer for read data
 * \param[in]   count			    Number if elements to read
 * \param[in]   stream			    File stream pointer
 *
 * \return                          EOF on error
 */
sfs_fsize_t sfs_fread(void* data, sfs_fsize_t count, sfs_fstream_t* stream);


/*! \brief          Write data to file
 *
 * \param[out]  data			    Pointer to data for writing
 * \param[in]   count			    Number if elements to write
 * \param[in]   stream			    File stream pointer
 *
 * \return                              EOF on error
 */
sfs_fsize_t sfs_fwrite(const void* data, sfs_fsize_t count, sfs_fstream_t* stream);


/*! \brief          Format file systme area of physical device
 *
 *                  The entire content of the physical drive is lost
 *                  and all available space is freed
 *
 * \return                              EOF on error
 */
int sfs_format_drive(void);


/*! \brief          Search for file
 *
 *                  Get next file that matches search string.
 *
 * \param[in]		search_string	    Search string of file to look for [basename.extention] '*' is allowed but only for entire basename or extention.
 * \param[out]	  	filename		    Name of file if found.
 * \param[in]		search_direction	Direction of search. SFS_FGET_RESTART just returns
 *
 * \return                              EOF if file not found
 */
int sfs_fget(const unsigned char* search_string,  unsigned char* filename, sfs_fget_t search_direction);


#ifdef SFS_USE_ERROR_STRINGS
/*! \brief          Get last error message
 *
 * \return                              Pointer to current error messsage (program memory string, \ref sfs_err_t)
 */
sfs_err_t sfs_err(void);
#endif


/*! \brief          Count number of files matching search string
 *
 * \param[in]		search_string	    Search string of file to look for [basename.extention] '*' is allowed but only for entire basename or extention.
 *
 * \return                              Number of files matching search string. EOF on error
 */
int sfs_fcount(const unsigned char* search_string);


/*! \brief         Initialize physical interface to storage medium
 *
 *                  This function must be defined by user since it is implementation dependent
 *
 * \return                              Must return EOF on error
 */
extern int sfs_hal_init(void);

#endif // __SFS_H__
/*EOF*/

// This file has been prepared for Doxygen automatic documentation generation.
/*! \file *********************************************************************
 *
 * \brief  Simple File System
 *
 *          The file system is implemented using a double linked list of file segments
 *          In most application it is convinient to make the segment size match some
 *          sort of parameters on the pysical medium such as flash page etc.
 *
 *          At the user defined place of the physical medium a file entry table is
 *          made. Every entry in the table holds key info and points to the first segment
 *          of the file (or, if not used, just zero). See \ref sfs_file_t.
 *          
 *          When the file system area of the physical medium is formatted, every
 *          file segment is linked into the freepool, and the entire entry table is
 *          written to zero.
 *
 *          When a file is created, a free (eq. zero) entry in the entry table is
 *          taken, key values of file (name, file start etc.) is stored in entry,
 *          and one file segment is allocated.
 *
 *          When a file is deleted, the entire chain of linked file segments is 
 *          connected to the tail of the freepool, and the entry is written to zero.
 *
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
 * $Id: fs.c 41490 2008-05-09 13:26:35Z hmyklebust $
 *
 * Copyright (c) 2008 , Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel’s Limited License Agreement (RZRaven Evaluation and Starter Kit). 
 *****************************************************************************/

/*========================= INCLUDES                 =========================*/
#include "sfs.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>


/*========================= MACROS                   =========================*/

//@{! Standard file handles
#define SFS_HANDLE_STDIN  0
#define SFS_HANDLE_STDOUT 1
#define SFS_HANDLE_STDERR 2
//@}

//@{! Set current error
#ifdef SFS_USE_ERROR_STRINGS
#define SFS_SET_ERROR_MSG(msg)   sfs_current_err_g = msg
#else
#define SFS_SET_ERROR_MSG(msg)
#endif
//@}

//! Re-initialize the file system
#define sfs_reinit()     sfs_deinit(); sfs_init()

//! Get a next file segment for reading
#define sfs_segment_get_rd(stream) sfs_segment_get(&stream->rd_ptr, &stream->rd_seg)

//! Get a next file segment for writing
#define sfs_segment_get_wr(stream) sfs_segment_get(&stream->wr_ptr, &stream->wr_seg)

//! Returned by \ref sfs_entry_find() if no entry found
#define SFS_ENTRY_NO_ENTRY (0xFFFFFFFF)

//! Current read position in file, relative to start of file
#define CURRENT_READ_POSITION() (stream->rd_seg*SFS_FILE_SEGMENT_SIZE + stream->rd_ptr%SFS_BLOCK_SIZE - SFS_FILE_SEGMENT_HEADER_SIZE)

//! Current write position in file, relative to start of file
#define CURRENT_WRITE_POSITION() (stream->wr_seg*SFS_FILE_SEGMENT_SIZE + stream->wr_ptr%SFS_BLOCK_SIZE - SFS_FILE_SEGMENT_HEADER_SIZE)
    

/*========================= TYPEDEFS                 =========================*/
#if SFS_MAX_FILE_HANDLES <= 8
    //! File handle type
    typedef uint8_t sfs_handle_t;
#elif SFS_MAX_FILE_HANDLES <= 16
    //! File handle type
    typedef uint16_t sfs_handle_t;
#elif SFS_MAX_FILE_HANDLES <= 32
    //! File handle type
    typedef uint32_t sfs_handle_t;
#else
    #error ("SFS_MAX_OPEN_STREAMS")
#endif

//! Error codes
typedef enum {
    SFS_ERR_MSG_FATAL,
    SFS_ERR_MSG_FILE_OPEN,
    SFS_ERR_MSG_FILE_CLOSED,
    SFS_ERR_MSG_FILE_NOT_FOUND,
    SFS_ERR_MSG_FILE_ALLREADY_EXISTS,
    SFS_ERR_MSG_NO_MORE_FILE_SEGMENTS,
    SFS_ERR_MSG_NO_MORE_FILE_ENTRIES,
    SFS_ERR_MSG_INVALID_HANDLE,
    SFS_ERR_MSG_ZERO_POINTER,
    SFS_ERR_MSG_TOO_MANY_OPEN_FILES,
    SFS_ERR_MSG_OUT_OF_DISK_SPACE,
    SFS_ERR_MSG_SYNTAX_ERROR,
    SFS_ERR_MSG_NO_ERR,
    SFS_ERR_MSG_COUNT
}sfs_errnumb_t; 

//! File struct. Used to build the file system entry table
typedef struct sfs_file{
    sfs_fname_t      basename[SFS_MAX_BASENAME_SIZE];
    sfs_ftype_t      extension[SFS_MAX_EXTENSION_SIZE];
    sfs_fsize_t      size;
    sfs_faddr_t      start;
    sfs_faddr_t      entry;
    sfs_faddr_t      end;
    sfs_fflags_t     flags;
} sfs_file_t;

/*========================= PUBLIC VARIABLES         =========================*/


/*========================= PRIVATE VARIABLES        =========================*/
#ifdef SFS_USE_ERROR_STRINGS
//! Error messages.Could be retrieved using \ref sfs_err()
static sfs_err_t sfs_error_codes[] = {
    "Fatal error",
    "File allready open",
    "File allready closed",
    "File not found",
    "File allready exists",
    "Out of file entries",
    "Invalid handle",
    "Zero pointer",
    "Out of disk space",
    "Syntax error",
    "No error"
};
#endif

#ifdef SFS_USE_ERROR_STRINGS
//! Flash pointer to last error message. Error message could be retrieved using \ref sfs_err()
static sfs_errnumb_t sfs_current_err_g;
#endif

//! Table for file streams
static sfs_fstream_t sfs_stream_tbl[SFS_MAX_OPEN_STREAMS];

//! Variable holding file handles
static sfs_handle_t FileHandles;

//! Freepool head pointer
static sfs_faddr_t sfs_freepool_head;

//! Freepool tail pointer
static sfs_faddr_t sfs_freepool_tail;

//! Flag indicating file system initialized
static bool sfs_initialized = false;

/*========================= PRIVATE FUNCTIONS        =========================*/

/*! \brief  Get the file entry address of a file providing its name
 *
 *          Detailed description
 *
 *  \param[in]  filename    Name of file to search for
 *
 *  \return     Entry address of file if found. If not, SFS_ENTRY_NO_ENTRY is returned
 */
static sfs_faddr_t  sfs_entry_find(const unsigned char* filename);


/*! \brief  Get a new file entry.
 *
 *          Searches for first free file entry in file entry table. If a free
 *          entry is found, it is written with the  values from the file struct
 *
 *  \param[in] file File pointer
 *
 *  \return EOF if no more free entries
 */
static int sfs_entry_alloc(sfs_file_t* file);


/*! \brief  Check if file is open
 *
 *          Searches for the file entry in the "open stream" table 
 *
 *  \param[in]  entry   Entry address of file
 *
 *  \return     Nothing
 */
static sfs_fstream_t* sfs_isopen(sfs_faddr_t entry);


/*! \brief  Get next file segment of stream
 *
 *          Updates read/write pointer and read/write segment index of file stream
 *
 *  \param[in,out] stream_ptr Pointer to read/write pointer of stream
 *  \param[in,out] stream_seg Pointer to read/write segment index of stream
 *
 *  \return EOF on fail or end of stream
 */
static int sfs_segment_get(sfs_findex_t* stream_ptr, int* stream_seg);


/*! \brief  Get next free file handle
 *
 *  \return     File handle. EOF on fail
 */
static int sfs_handle_get(void);


/*! \brief  Free file handle
 *
 *  \param[in]  handle    Handle of file to tree
 *
 *  \return     EOF on fail
 */
static int sfs_handle_free(sfs_fhandle_t handle);


/*! \brief  Get a new file segment
 *
 *          Updates write pointer and write segment index of file stream
 *
 *  \param[in,out]  stream Pointer to stram
 *
 *  \return EOF on fail
 */
static int sfs_segment_alloc(sfs_fstream_t* stream);


/*! \brief  Free a file entry
 *
 *          Remove file entry from file entry list and connect all file segments
 *          to the freepool tail
 *
 *  \param[in]  entry   File entry of file to delete
 *
 *  \return     EOF on fail
 */
static int sfs_free(sfs_faddr_t entry);


/*! \brief  Open a new file strem
 *
 *  \param[in]  file    Pointer to file entry struct
 *  \param[in]  mode    Open mode
 *
 *  \return     Pointer to file stream
 */
static sfs_fstream_t* open_(sfs_file_t* file, uint8_t mode);


/*! \brief  Gets next/previous file entry
 *
 *          The purpose of this funtion is to increase/decrease the entry address
 *          to next entry and wrap if start/end of entry table is reached.
 *
 *  \param[in]  entry    Current file entry address
 *  \param[in]  count    Number of entries to skip
 *  \param[in]  search_direction    Direction of search
 *
 *  \return     Nothing
 */
static sfs_faddr_t sfs_entry_get(sfs_faddr_t entry, int count, sfs_fget_t search_direction);


/*! \brief  Get next free segment
 *
 *          Returns a pointer to the header of next free segment. The segment is
 *          removed from the freepool and the freepool pointer is updated
 *
 *  \return     Address of new segment. If no more free segments, 0x00000000 is returned
 */
static sfs_faddr_t free_segment_get(void);


/*! \brief  This function splits the input filename into basename and extension
 *
 *          The '.' separates the basename and extension of the filename.
 *
 *  \param[in]  filename    Provided filename. Doesn't need to be zero terminated if of max length
 *  \param[out] basename    Generated basename
 *  \param[out] extension   Generated extention
 *
 *  \return     EOF if file name too long 
 */
static int split_filename(const unsigned char* filename, unsigned char* basename, unsigned char* extension);


/*! \brief  This function concatenates the input basename and extension into a single filename
 *
 *          A '.' is added between the basename and extension when making the filename.
 *
 *  \param[out] filename    Generated filename
 *  \param[in]  basename    Provided basename. Doesn't need to be zero terminated if of max length
 *  \param[in]  extension   Provided extention. Doesn't need to be zero terminated if of max length
 *
 *  \return     EOF if file name too long 
 */
static int make_filename(unsigned char* filename, const unsigned char* basename, const unsigned char* extension);

/*========================= IMPLEMENTATION           =========================*/
int sfs_init(void)
{
    // Initialize HW
    sfs_hal_init();

    // Fill stream table with non-valid entries
    for (int i=0; i<SFS_MAX_OPEN_STREAMS; ++i) {
        sfs_stream_tbl[i].entry = 0xFFFFFFFF;
    }

    // take 3 first handles (0,1 and 2, reserved for the std streams stdin, stdout and stderr)
    // should be moved from sfs_init!!!
    FileHandles = 0x07;

    // Read the value of the freepool head pointer, stored on the physical medium
    sfs_hal_read(SFS_FREE_SEGMENT_HEAD_PTR_ADR, SFS_FREE_SEGMENT_HEAD_PTR_SIZE, (uint8_t*)&sfs_freepool_head);

    // Indicatate initialized with no error
    SFS_SET_ERROR_MSG(SFS_ERR_MSG_NO_ERR);
    sfs_initialized = true;
    return 0;
}

void sfs_deinit(void)
{
    sfs_initialized = false;
}

sfs_fstream_t* sfs_fopen(const unsigned char* filename, uint8_t mode)
{
    // Locals
    sfs_fstream_t*   stream = (sfs_fstream_t*)NULL;
    sfs_file_t       file;

    // File system not initialized?
    if (sfs_initialized == false) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_FATAL);
    // File exists?
    } else if ((file.entry = sfs_entry_find(filename)) != SFS_ENTRY_NO_ENTRY) {
        // Read file entry
        sfs_hal_read(file.entry, SFS_ENTRY_SIZE, (uint8_t*)&file);

        // File allready open?
        if ((stream = sfs_isopen(file.entry)) != NULL) {
            uint8_t CommonModes = stream->mode & mode;

            // Check flags to determin allerady opened for read/write
            // It is possible to open a file for reading in one instance and writing in another
            if ((CommonModes & SFS_OPEN_MODE_RD) || (CommonModes & SFS_OPEN_MODE_WR)) {
                SFS_SET_ERROR_MSG(SFS_ERR_MSG_FILE_OPEN);
                stream = (sfs_fstream_t*)NULL;
            }
        // File exists but not open, -> open file
        } else {
            stream = open_(&file, mode);
        }
    // If file does not exists, but "create file" mode
    } else if (mode & SFS_OPEN_MODE_CREATE) {
        if (sfs_fcreate(filename) == EOF) {
            SFS_SET_ERROR_MSG(SFS_ERR_MSG_FATAL);
        } else {
            // If entry created successfully, read entry
            if ((file.entry = sfs_entry_find(filename)) == SFS_ENTRY_NO_ENTRY) {
                SFS_SET_ERROR_MSG(SFS_ERR_MSG_FATAL);
            } else {
                sfs_hal_read(file.entry, SFS_ENTRY_SIZE, (uint8_t*)&file);
        
                // Open created file
                stream = open_(&file, mode);
            }
        }
    // File doesn't exists and "no create" mode, -> "File not found"
    } else {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_FILE_NOT_FOUND);
    }
    
    // Return stream pointer
    return stream;
}

int sfs_fclose(sfs_fstream_t* stream)
{
    int status = EOF;
    
    // File not open?
    if (sfs_handle_free(stream->handle) == EOF) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_INVALID_HANDLE);
    // Stream pointer zero pointer?
    } else if (stream == NULL) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_ZERO_POINTER);
    // Stream pointer not zero, update file size in entry table
    } else {
        // Update file size in file entry
        sfs_hal_write(stream->entry + SFS_ENTRY_SIZE_OFFSET, SFS_ENTRY_SIZE_SIZE, (uint8_t*)&stream->size);
    
        // Update file stream pointer to indicate file closed
        stream->mode = 0;
        stream->wr_ptr = 0;
        stream->rd_ptr = 0;
        stream->entry = 0xFFFFFFFF;
        stream->handle = 0;
        
        // status OK
        status = 0;
    }
       
    return status;
}

int sfs_fputc(const unsigned char c, sfs_fstream_t* stream)
{
    // Write one byte at end of file stream
    int write_count =sfs_fwrite((void const*)&c, sizeof(unsigned char), stream);
    
    // Return number of bytes written
    return write_count;
}

int sfs_fgetc(sfs_fstream_t* stream)
{
    // Read one byte file stream
    int read_byte;
    sfs_fread((void*)&read_byte, sizeof(int), stream);
    
    // Return read byte (Might be EOF)
    return read_byte;
}

sfs_fsize_t sfs_fread(void* data, sfs_fsize_t count, sfs_fstream_t* stream)
{
    uint8_t* sfs_data = (uint8_t*)data;
    
    sfs_size_t read_byte_count = 0;
    sfs_size_t request_byte_count = count;
    sfs_size_t bytes_to_read;
    
    // Don't read beond end of file
    bytes_to_read = ((CURRENT_READ_POSITION() + count) > stream->size) ? stream->size - CURRENT_READ_POSITION() : request_byte_count;

    // If read operation crosses segment bounds
    if ((stream->rd_ptr+request_byte_count)/SFS_BLOCK_SIZE != stream->rd_ptr/SFS_BLOCK_SIZE) {
        // First read rest of data from current segment, update file pointer and counter for read bytes
        sfs_size_t read_byte_count_current_seg = SFS_BLOCK_SIZE - stream->rd_ptr%SFS_BLOCK_SIZE;
        sfs_size_t bytes_read_from_current_seg = sfs_hal_read(stream->rd_ptr, read_byte_count_current_seg, sfs_data);
        
        // Update file pointer with number of bytes actually read from current file segment
        stream->rd_ptr += bytes_read_from_current_seg;
        
        // Update byte counter with number of bytes actually read from current file segment
        read_byte_count += bytes_read_from_current_seg;

        // Subtract number of bytes read from current segment from "bytes_to_read" before reading from next
        bytes_to_read -= bytes_read_from_current_seg;

        // Read data from next segment(s)
        while (bytes_to_read){
            //  On error, return number of bytes read till now
            if (sfs_segment_get_rd(stream) == EOF) {
                return read_byte_count;
            }
            
            // Read
            sfs_size_t bytes_read = sfs_hal_read(stream->rd_ptr, bytes_to_read%SFS_FILE_SEGMENT_SIZE, sfs_data);
            
            // Update file pointer with number of bytes actually read from current file segment
            stream->rd_ptr += bytes_read;
            
            // Update byte counter with number of bytes actually read from current file segment
            read_byte_count += bytes_read;
            
            // Subtract number of bytes read from current segment from "bytes_to_read" before reading from next
            bytes_to_read -= bytes_read;
        }
    } else {
        // Read
        sfs_size_t bytes_read = sfs_hal_read(stream->rd_ptr, bytes_to_read, sfs_data);
        
        // Update file pointer with number of bytes actually read
        stream->rd_ptr += bytes_read;
        
        // Update byte counter with number of bytes actually read
        read_byte_count += bytes_to_read;
    }
    
    // If read pointer points exactly at start of next segment, update read- and segment pointer
    if (!(stream->rd_ptr%SFS_BLOCK_SIZE)) {
        // Move read pointer back equal size of the new segment
        stream->rd_ptr -= SFS_BLOCK_SIZE;
        
        //  Get a new segment. If error return number of bytes read till now
        if (sfs_segment_get_rd(stream) == EOF) {
            return read_byte_count;
        }
    }
    
    // Return number of bytes read
    return read_byte_count;
}

sfs_fsize_t sfs_fwrite(const void* data, sfs_fsize_t count, sfs_fstream_t* stream)
{
    sfs_size_t nmb = count;
    uint8_t* sfs_data = (uint8_t*)data;

    // If write operation crosses segment bounds
    if ((stream->wr_ptr+count)/SFS_BLOCK_SIZE != stream->wr_ptr/SFS_BLOCK_SIZE) {

        // Calculate byte count to current segment
        sfs_size_t temp = SFS_BLOCK_SIZE - stream->wr_ptr%SFS_BLOCK_SIZE;
        
        // Write data to current segment
        stream->wr_ptr += sfs_hal_write(stream->wr_ptr, temp, sfs_data);

        // Update variables
        count -= temp;
        sfs_data += temp;

        // Write data to next segment(s)
        while (count){
            // Allocate or get a new segment? This will update wr_ptr to point to
            // first data field of next segment
            if (stream->wr_seg < stream->size/SFS_FILE_SEGMENT_SIZE) {
                sfs_segment_get_wr(stream);
            } else {
                sfs_segment_alloc(stream);
            }

            stream->wr_ptr += sfs_hal_write(stream->wr_ptr, count%SFS_FILE_SEGMENT_SIZE, sfs_data);
            count -= count%SFS_FILE_SEGMENT_SIZE;
        }
    }
    else {
        stream->wr_ptr += sfs_hal_write(stream->wr_ptr, count, sfs_data);
    }
    
    // If pointer outside segment
    if (!(stream->wr_ptr%SFS_BLOCK_SIZE)) {
        stream->wr_ptr -= SFS_BLOCK_SIZE; // move pointer back into valid segment
        if (stream->wr_seg < stream->size/SFS_FILE_SEGMENT_SIZE) {
            sfs_segment_get_wr(stream);
        } else {
            sfs_segment_alloc(stream);
        }
    }
    
    // update file size
    stream->size = CURRENT_WRITE_POSITION();
    
    // Return number of bytes written
    return nmb;
}


int sfs_fflush(sfs_fstream_t* stream)
{
    sfs_file_t file;

    // Read file information from physical medium. Return EOF on error
    if (sfs_hal_read(stream->entry, SFS_ENTRY_SIZE, (uint8_t*)&file) != SFS_ENTRY_SIZE) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_FATAL);
        return EOF;
    }

    // Update stream pointers read- and write pointers with initial values
    stream->wr_ptr = file.start + SFS_FILE_SEGMENT_HEADER_SIZE;
    stream->rd_ptr = file.start + SFS_FILE_SEGMENT_HEADER_SIZE;
    stream->rd_seg = 0;
    stream->wr_seg = 0;

    // Return OK
    return 0;
}

int sfs_fcreate(const unsigned char *filename)
{
    sfs_fseg_t segment;  // Temp
    sfs_faddr_t NewSegmentAdr;
    
    // File with same name allready exists?
    if (sfs_entry_find(filename) != SFS_ENTRY_NO_ENTRY) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_FILE_ALLREADY_EXISTS);
        return EOF;
    }
    
    // Take next free file segment, return if data flash full
    if ((NewSegmentAdr = free_segment_get()) == 0x00000000) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_NO_MORE_FILE_SEGMENTS);
        return EOF;
    }
    
    // Initialize the new file entry struct 
    sfs_file_t file;
    split_filename(filename, file.basename, file.extension);
    file.size   = 0;
    file.flags  = 0;
    file.start  = NewSegmentAdr;
    file.end    = NewSegmentAdr;
    
    // Update header in new file segment. The .prev pointer shal not point to file entry
    // all though the file entry .start pointer will point to this segment
    segment.next = 0x00000000;
    segment.prev = 0x00000000;
    sfs_hal_write(NewSegmentAdr, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    
    // Get a new file entry in the entry table. The address of first segment (above) will
    // written to this entry struct
    if (sfs_entry_alloc(&file) == EOF) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_NO_MORE_FILE_ENTRIES);
        return EOF;
    }
    
    // Return OK
    return 0;
}

int sfs_remove(const unsigned char* filename)
{
    sfs_fstream_t *StreamToRemove;
    int status = EOF;
    
    // Search for file entry in entry table
    sfs_faddr_t entry = sfs_entry_find(filename);
    
    // File not found?
    if (entry == SFS_ENTRY_TBL_END) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_FILE_NOT_FOUND);
    // File allready open?
    } else if ((StreamToRemove = sfs_isopen(entry)) != NULL) {
        if (sfs_fclose(StreamToRemove)) {
            SFS_SET_ERROR_MSG(SFS_ERR_MSG_FATAL);
        }
    // Everyting ok
    } else {
        // Free data space and file entry occupied by file
        sfs_free(entry);
        
        // status OK
        status = 0;
    }

    // Return status
    return status;
}

int sfs_format_drive(void)
{
    sfs_fseg_t segment;

    // Erase entire entry table (All file entries will be lost!)
    sfs_hal_fill(SFS_ENTRY_TBL_START, SFS_ENTRY_SIZE*SFS_ENTRY_CNT, 0x00);
    
    // Make freepool pointer point to first available file segment
    sfs_freepool_head = SFS_FILE_SECT_START;
    sfs_hal_write(SFS_FREE_SEGMENT_HEAD_PTR_ADR, SFS_FREE_SEGMENT_HEAD_PTR_SIZE, (uint8_t*)&sfs_freepool_head);

    // Make freepool tail point to last available file segment
    sfs_freepool_tail = SFS_FILE_SECT_LAST;
    sfs_hal_write(SFS_FREE_SEGMENT_TAIL_ADR, SFS_FREE_SEGMENT_TAIL_SIZE, (uint8_t*)&sfs_freepool_head);
    
    // Link all available pages into the freepool
    segment.next = SFS_FILE_SECT_START + SFS_BLOCK_SIZE;
    segment.prev = SFS_FILE_SECT_START - SFS_BLOCK_SIZE;
    for (sfs_faddr_t file_segment = SFS_FILE_SECT_START; file_segment < SFS_FILE_SECT_END; file_segment += SFS_BLOCK_SIZE) {
        sfs_hal_write(file_segment, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
        segment.next += SFS_BLOCK_SIZE;
        segment.prev += SFS_BLOCK_SIZE;
    }
    
    /* The previous loop have given the the first segment's .prev pointer and the
     * last segment's .next pointer wrong values. These will be updated below
    */

    // Write .prev pointer of first segment with a zero pointer
    sfs_hal_read(sfs_freepool_head, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    segment.prev = 0x00000000;
    sfs_hal_write(sfs_freepool_head, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    
    // Write .next pointer of last segment with a zero pointer
    sfs_hal_read(sfs_freepool_tail, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    segment.next = 0x00000000;
    sfs_hal_write(sfs_freepool_tail, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    
    // Re-initialize entire file system (including HW reset)
    sfs_reinit();
    
    // Return OK
    return 0;
}

int sfs_fcount(const unsigned char* search_string)
{
    int file_count = 0;
    uint8_t start_name[SFS_MAX_FILENAME_BUFFER_SIZE];
    uint8_t name[SFS_MAX_FILENAME_BUFFER_SIZE];
    
    // 
    sfs_fget(search_string, start_name, SFS_FGET_RESTART);
    if (sfs_fget(search_string, start_name, SFS_FGET_FWD) != EOF) {
        do {
            file_count++;
            if (sfs_fget(search_string, name, SFS_FGET_FWD) == EOF) {
                break;
            }
        } while (strncmp((const char*)start_name, (const char*)name, SFS_MAX_FILENAME_SIZE) != 0);
    }
    
    return file_count;
}

int sfs_fget(const unsigned char* search_string,  unsigned char* filename, sfs_fget_t search_direction)
{
    sfs_file_t file;
    static sfs_faddr_t entry = SFS_ENTRY_TBL_START; // last entry accessed (static)
    static sfs_fget_t previous_direction;
    unsigned char search_basename[SFS_MAX_BASENAME_BUFFER_SIZE];
    unsigned char search_extension[SFS_MAX_EXTENSION_BUFFER_SIZE];
    sfs_faddr_t StartEntry;
    
    // New access
    if (search_direction == SFS_FGET_RESTART) {
        entry = SFS_ENTRY_TBL_START;
        return 0;
    }
    
    //Set default as emtpy string
    filename[0] = '\0';

    // Get filename and extension from search string
    if (split_filename(search_string, search_basename, search_extension) == EOF) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_SYNTAX_ERROR);
        return EOF;
    }
    
    // If search direction has changed, increase/decrease next entry by 2 to avoid returning the same filename
    if ((search_direction != previous_direction) && (search_direction != SFS_FGET_RESTART)) {
        entry = sfs_entry_get(entry, 2, search_direction);
    }
    
    // Save start entry
    StartEntry = entry;
    
    // Save search direction
    previous_direction = search_direction;
    
    do {
        // Read file entry
        sfs_hal_read(entry, SFS_ENTRY_SIZE, (uint8_t*)&file);
        
        // If valid file entry, parse it
        if (file.entry == entry) {
            // get next file entry
            entry = sfs_entry_get(entry, 1, search_direction);
            
            // Check filename
            if (search_basename[0]=='*') {
                if (search_extension[0]=='*') {
                    make_filename(filename, file.basename, file.extension);
                    return 0;
                } else if (!strncmp((char const*)&file.extension, (const char*)search_extension, SFS_MAX_EXTENSION_SIZE)) {
                    make_filename(filename, file.basename, file.extension);
                    return 0;
                }
            } else if (!strncmp((const char*)file.basename, (const char*)search_basename, SFS_MAX_BASENAME_SIZE)) {
                if (search_extension[0]=='*') {
                    make_filename(filename, file.basename, file.extension);
                    return 0;
                } else if (!strncmp((const char*)file.extension, (const char*)search_extension, SFS_MAX_EXTENSION_SIZE)) {
                    make_filename(filename, file.basename, file.extension);
                    return 0;
                }
            }
        } else {
            // get next file entry
            entry = sfs_entry_get(entry, 1, search_direction);
        }
    }
    while(entry != StartEntry); // search through entry list only one time
    
    // If execution gets here, requested file is not found
    SFS_SET_ERROR_MSG(SFS_ERR_MSG_FILE_NOT_FOUND);
    return EOF;
}

static sfs_fstream_t* sfs_isopen(sfs_faddr_t entry)
{
    for( sfs_fstream_t* stream = &sfs_stream_tbl[3];
         stream != &sfs_stream_tbl[SFS_MAX_OPEN_STREAMS];
         stream++)
    {
         if (stream->entry == entry) {
             return stream;
         }
    }
    return (sfs_fstream_t*)NULL;
}

#ifdef SFS_USE_ERROR_STRINGS
sfs_err_t sfs_err(void)
{
    // Return last error message
    return sfs_error_codes[sfs_current_err_g];
    
    // Reset error flag
    SFS_SET_ERROR_MSG(SFS_ERR_MSG_NO_ERR);
}
#endif

static sfs_faddr_t sfs_entry_get(sfs_faddr_t entry, int count, sfs_fget_t search_direction) {
    
    while(count--) {
        // read next/previous file entry from data flash depending on search direction
        if (search_direction == SFS_FGET_FWD) {
            if ((entry += SFS_ENTRY_SIZE) == SFS_ENTRY_TBL_END) {
                entry = SFS_ENTRY_TBL_START;
            }
        } else /*search_direction == SFS_FGET_REV*/{
            if ((entry -= SFS_ENTRY_SIZE) < SFS_ENTRY_TBL_START) {
                entry = SFS_ENTRY_TBL_END - SFS_ENTRY_SIZE;
            }
        }
    }
    
    return entry;
}

static int sfs_segment_alloc(sfs_fstream_t* stream)
{
    sfs_faddr_t last_seg_ptr =  stream->wr_ptr - stream->wr_ptr%SFS_BLOCK_SIZE;
    sfs_fseg_t segment;
    sfs_faddr_t NewSegmentAdr;
    
    // Take next free file segment, return if data flash full
    if ((NewSegmentAdr = free_segment_get()) == 0x00000000) {
        return EOF;
    }
    
    // Update header in new segment. The .prev pointer shall point to previous
    // segment in chain
    segment.next = 0x00000000; // end of chain
    segment.prev = last_seg_ptr; // previous segment (from argument)
    sfs_hal_write(NewSegmentAdr, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    
    // Update previous segment's .next field to point to this segment
    sfs_hal_read(last_seg_ptr, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    segment.next = NewSegmentAdr;
    sfs_hal_write(last_seg_ptr, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    
    // update pointer to point to first valid data location on newly allocated segment
    stream->wr_ptr = NewSegmentAdr + SFS_FILE_SEGMENT_HEADER_SIZE;
    
    // Update segment counter
    stream->wr_seg++;
        
    return 0;
}

static int sfs_segment_get(sfs_findex_t* stream_ptr, int* stream_seg)
{
    sfs_fseg_t segment;
    
    // Calculate segment header address of current segment
    sfs_faddr_t seg_hdr_addr = *stream_ptr - ((*stream_ptr)%SFS_BLOCK_SIZE);

    // Read the the current segment header
    sfs_hal_read(seg_hdr_addr, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);

    // .next == 0x000000 indicates EOF
    if (segment.next == 0x00000000) {
        return EOF;
    }

    // Set pointer to first data field of next segment
    *stream_ptr = segment.next + SFS_FILE_SEGMENT_HEADER_SIZE;

    // Update segment counter
    (*stream_seg)++;
    
    return 0;
}

static sfs_faddr_t sfs_entry_find(const unsigned char* filename)
{
    unsigned char basename[9];
    unsigned char extension[4];
    
    /* Use a continous buffer for the file name in case the input file name breaks the
     * rules of max length of 8 for first- and 3 for last name
     */
    unsigned char SearchBuffer[SFS_MAX_FILENAME_BUFFER_SIZE];
    unsigned char* search_basename = &SearchBuffer[0];
    unsigned char* search_extension = &SearchBuffer[SFS_MAX_BASENAME_BUFFER_SIZE];

    // Split file name into basename and extension
    split_filename(filename, search_basename, search_extension);
    
    // Search through entire file entry list
    sfs_faddr_t entry = SFS_ENTRY_TBL_START;
    do {
        // Read file basename from current entry 
        sfs_hal_read(entry + SFS_ENTRY_BASENAME_OFFSET, SFS_ENTRY_BASENAME_BUFFER_SIZE, (uint8_t*)basename);
        
        // Entry basename equal search basename?
        if (!strncmp((const char*)basename, (const char*)search_basename, SFS_ENTRY_BASENAME_BUFFER_SIZE)) {
            // Read file extention from current entry 
            sfs_hal_read(entry + SFS_ENTRY_EXTENSION_OFFSET, SFS_ENTRY_EXTENSION_BUFFER_SIZE, (uint8_t*)extension);
            
            // Entry extention equal search extention?
            if (!strncmp((const char*)extension, (const char*)search_extension, SFS_ENTRY_EXTENSION_BUFFER_SIZE)) {
                // Return entry address if match
                return entry;
            }
        }
        
        // Move address to next file entry
        entry += SFS_ENTRY_SIZE;
    }
    while(entry < SFS_ENTRY_TBL_END); // End of entry table?

    // Return EOF if file entry not found
    return SFS_ENTRY_NO_ENTRY;
}


static int sfs_entry_alloc(sfs_file_t* file)
{
    // Search through entire file entry table, firs one with a '\x0' as first byte (free) is allocated
    // and the file data is written into entry.
    for ( file->entry = SFS_ENTRY_TBL_START; file->entry <SFS_ENTRY_TBL_END; file->entry += SFS_ENTRY_SIZE) {
        uint8_t Temp;
        sfs_hal_read(file->entry, 1, &Temp);
        if (Temp == '\x0') {
            sfs_hal_write(file->entry, SFS_ENTRY_SIZE, (uint8_t*)file);
            return 0;
        }
    }
    return EOF;
}

static int sfs_free(sfs_faddr_t entry)
{
    sfs_file_t file;
    sfs_fseg_t segment;

    // Read file entry
    sfs_hal_read(entry, SFS_ENTRY_SIZE, (uint8_t*)&file);

    // Link the file segment chain into the freepool tail
    sfs_hal_write(sfs_freepool_tail + SFS_SEG_HDR_NEXT_OFFSET, SFS_SEG_HDR_NEXT_SIZE, (uint8_t*)&file.start);

    // Scan through the newly connected chain to find the end while continously updating the freepool tail pointer
    sfs_freepool_tail = file.start;
    do {
        // Read segment
        sfs_hal_read(sfs_freepool_tail, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
        
        // Update freepool tail pointer if more segments in chain (.next pointer not equal zero) 
        sfs_freepool_tail = (segment.next != 0x00000000) ? segment.next : sfs_freepool_tail;
    } while(segment.next != 0x00000000);
    
    // Delete the file entry
    sfs_hal_fill(entry, SFS_ENTRY_SIZE, 0x00);
    
    return 0;
}

static int sfs_handle_get(void){
    // Search through all bits in "FileHandles" for a cleared bit.
    // Bit set means handle taken
    int handle = 0;
    while (handle <= SFS_MAX_OPEN_STREAMS) {
        if ((FileHandles & (1<<handle)) == 0) {
            FileHandles |= (1<<handle);
            return handle;
        } else {
             ++handle;
        }
    }
    return EOF;
}

static int sfs_handle_free(sfs_fhandle_t handle)
{
    if (!(FileHandles & (1<<handle))) {
     //   ERROR("Freeing unused handle(%d)",handle);
        return EOF;
    } else {
        FileHandles &= ~(1<<handle);
        return 0;
    }
}

static sfs_fstream_t* open_(sfs_file_t* file, uint8_t mode)
{
    sfs_fhandle_t handle;

    // Get pointer to an available sfs_fstream_t struct,
    // return if too many open files
    if ((handle = sfs_handle_get()) == EOF) {
        SFS_SET_ERROR_MSG(SFS_ERR_MSG_TOO_MANY_OPEN_FILES);
        return (sfs_fstream_t*)NULL;
    } else {// If not, Initialize sfs_fstream_t struct and return a pointer
        sfs_stream_tbl[handle].handle = handle;
        sfs_stream_tbl[handle].wr_ptr = file->start + SFS_FILE_SEGMENT_HEADER_SIZE;
        sfs_stream_tbl[handle].rd_ptr = file->start + SFS_FILE_SEGMENT_HEADER_SIZE;
        sfs_stream_tbl[handle].mode = mode;
        sfs_stream_tbl[handle].entry = file->entry;
        sfs_stream_tbl[handle].size = file->size;
        sfs_stream_tbl[handle].rd_seg = 0;
        sfs_stream_tbl[handle].wr_seg = 0;

        return &sfs_stream_tbl[handle];
    }
}

static sfs_faddr_t free_segment_get(void)
{
    sfs_fseg_t segment;
    
    // File section of data flash full?
    sfs_hal_read(sfs_freepool_head, SFS_SEG_HDR_SIZE, (uint8_t*)&segment);
    if (segment.next == 0x00000000) {
        return 0x00000000;
    }
    
    // Take next free file segment
    sfs_faddr_t NewSegmentAdr = sfs_freepool_head;
    
    // Update free segment head pointer
    sfs_freepool_head = segment.next;
    sfs_hal_write(SFS_FREE_SEGMENT_HEAD_PTR_ADR, SFS_FREE_SEGMENT_HEAD_PTR_SIZE, (uint8_t*)&sfs_freepool_head);
    
    // Update the .prev field in the new free segment head to a zero pointer
    sfs_faddr_t  Addr = 0x00000000;
    sfs_hal_write(sfs_freepool_head+SFS_SEG_HDR_PREV_OFFSET, SFS_SEG_HDR_PREV_SIZE, (uint8_t*)&Addr);
    
    return NewSegmentAdr;
}

static int split_filename(const unsigned char* filename, unsigned char* basename, unsigned char* extension)
{
    unsigned char* ptr = basename;
    
    // Start by reading first part of filename into basename 
    for (int i=0; i<= SFS_MAX_FILENAME_SIZE; i++) {
        // If a dot is found, terminate basename and move destignation pointer to extension
        if (*filename == '.') {
            *ptr = '\x0';
            ptr = extension;
            filename++;
        // If a end of string is found, zero terminate extension and return
        } else if (*filename == '\x0') {
            *ptr = '\x0';
            return 0;
        }
        *(ptr++) = *(filename++);
    }
    
    // If a file extension is of max size, zero terminate ptr (now pointing at extension[3])
    *ptr = '\x0';
    
    return 0;
}

static int make_filename(unsigned char* filename, const unsigned char* basename, const unsigned char* extension)
{
    // Copy basename
    for (int i=0; i < SFS_MAX_BASENAME_SIZE; i++) {
        if (*basename == '\x0') {
            break;
        } else {
            *(filename++) = *(basename++);
        }
    }
    
    // And add a dot '.'
    *(filename++) = '.';
    
    // Copy extension
    for (int i=0; i < SFS_MAX_EXTENSION_SIZE; i++) {
        if (*(extension) == '\x0') {
            break;
        }
        *(filename++) = *(extension++);
    }
    
    // Zero terminate filename
    *filename = '\x0';
    
    return 0;
}
/*EOF*/


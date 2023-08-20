/*
 * This file is part of libbluray
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2009-2010  John Stebbins
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * \brief Filesystem interface
 *
 * File access wrappers can be used to bind libbluray to external filesystem.
 * Typical use case would be playing BluRay from network filesystem.
 */

#ifndef BD_FILESYSTEM_H_
#define BD_FILESYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * File access
 */
typedef struct bd_file_s BD_FILE_H;
struct bd_file_s
{
    /** Reserved for BD_FILE_H implementation use.
     *  Implementation can store here ex. file handle, FILE*, ...
     */
    void* internal;

    /**
     *  Close file
     *
     *  @param file BD_FILE_H object
     */
    void    (*close) (BD_FILE_H *file);

    /**
     *  Reposition file offset
     *
     *  - SEEK_SET: seek to 'offset' bytes from file start
     *  - SEEK_CUR: seek 'offset' bytes from current position
     *  - SEEK_END: seek 'offset' bytes from file end
     *
     *  @param file BD_FILE_H object
     *  @param offset byte offset
     *  @param origin SEEK_SET, SEEK_CUR or SEEK_END
     *  @return current file offset, < 0 on error
     */
    int64_t (*seek)  (BD_FILE_H *file, int64_t offset, int32_t origin);

    /**
     *  Get current read or write position
     *
     *  @param file BD_FILE_H object
     *  @return current file offset, < 0 on error
     */
    int64_t (*tell)  (BD_FILE_H *file);

    /**
     *  Check for end of file
     *
     *  - optional, currently not used
     *
     *  @param file BD_FILE_H object
     *  @return 1 on EOF, < 0 on error, 0 if not EOF
     */
    int     (*eof)   (BD_FILE_H *file);

    /**
     *  Read from file
     *
     *  @param file BD_FILE_H object
     *  @param buf buffer where to store the data
     *  @param size bytes to read
     *  @return number of bytes read, 0 on EOF, < 0 on error
     */
    int64_t (*read)  (BD_FILE_H *file, uint8_t *buf, int64_t size);

    /**
     *  Write to file
     *
     *  Writing 0 bytes can be used to flush previous writes and check for errors.
     *
     *  @param file BD_FILE_H object
     *  @param buf data to be written
     *  @param size bytes to write
     *  @return number of bytes written, < 0 on error
     */
    int64_t (*write) (BD_FILE_H *file, const uint8_t *buf, int64_t size);
};

/**
 * Directory entry
 */

typedef struct
{
    char    d_name[256];  /**< Null-terminated filename */
} BD_DIRENT;

/**
 * Directory access
 */

typedef struct bd_dir_s BD_DIR_H;
struct bd_dir_s
{
    void* internal; /**< reserved for BD_DIR_H implementation use */

    /**
     *  Close directory stream
     *
     *  @param dir BD_DIR_H object
     */
    void (*close)(BD_DIR_H *dir);

    /**
     *  Read next directory entry
     *
     *  @param dir BD_DIR_H object
     *  @param entry BD_DIRENT where to store directory entry data
     *  @return 0 on success, 1 on EOF, <0 on error
     */
    int (*read)(BD_DIR_H *dir, BD_DIRENT *entry);
};

/**
 *  Open a file
 *
 *  Prototype for a function that returns BD_FILE_H implementation.
 *
 *  @param filename name of the file to open
 *  @param mode string starting with "r" for reading or "w" for writing
 *  @return BD_FILE_H object, NULL on error
 */
typedef BD_FILE_H* (*BD_FILE_OPEN)(const char* filename, const char *mode);

/**
 *  Open a directory
 *
 *  Prototype for a function that returns BD_DIR_H implementation.
 *
 *  @param dirname name of the directory to open
 *  @return BD_DIR_H object, NULL on error
 */
typedef BD_DIR_H* (*BD_DIR_OPEN) (const char* dirname);

/**
 *  Register function pointer that will be used to open a file
 *
 * @deprecated Use bd_open_files() instead.
 *
 * @param p function pointer
 * @return previous function pointer registered
 */
BD_FILE_OPEN bd_register_file(BD_FILE_OPEN p);

/**
 *  Register function pointer that will be used to open a directory
 *
 * @deprecated Use bd_open_files() instead.
 *
 * @param p function pointer
 * @return previous function pointer registered
 */
BD_DIR_OPEN bd_register_dir(BD_DIR_OPEN p);

#ifdef __cplusplus
}
#endif

#endif /* BD_FILESYSTEM_H_ */

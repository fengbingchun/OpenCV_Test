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
 * \brief Log control and capture
 *
 * Logging level can be changed with function bd_set_debug_mask() or environment variable BD_DEBUG_MASK.
 * Default is to log only errors and critical messages (DBG_CRIT).
 *
 * Application can capture log messages with bd_set_debug_handler().
 * Messages can be written to a log file with BD_DEBUG_FILE environment variable.
 * By default messages are written to standard error output.
 */

#ifndef BD_LOG_CONTROL_H_
#define BD_LOG_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * Flags for log filtering.
 */
typedef enum debug_mask_enum {
    DBG_RESERVED   = 0x00001, /*   (reserved) */
    DBG_CONFIGFILE = 0x00002, /*   (reserved for libaacs) */
    DBG_FILE       = 0x00004, /*   (reserved for libaacs) */
    DBG_AACS       = 0x00008, /*   (reserved for libaacs) */
    DBG_MKB        = 0x00010, /*   (reserved for libaacs) */
    DBG_MMC        = 0x00020, /*   (reserved for libaacs) */
    DBG_BLURAY     = 0x00040, /**< BluRay player */
    DBG_DIR        = 0x00080, /**< Directory access */
    DBG_NAV        = 0x00100, /**< Database files (playlist and clip info) */
    DBG_BDPLUS     = 0x00200, /*   (reserved for libbdplus) */
    DBG_DLX        = 0x00400, /*   (reserved for libbdplus) */
    DBG_CRIT       = 0x00800, /**< **Critical messages and errors** (default) */
    DBG_HDMV       = 0x01000, /**< HDMV virtual machine execution trace */
    DBG_BDJ        = 0x02000, /**< BD-J subsystem and Xlet trace */
    DBG_STREAM     = 0x04000, /**< m2ts stream trace */
    DBG_GC         = 0x08000, /**< graphics controller trace */
    DBG_DECODE     = 0x10000, /**< PG / IG decoders, m2ts demuxer */
    DBG_JNI        = 0x20000, /**< JNI calls */
} debug_mask_t;

/**
 *  Log a message
 *
 *  @param msg Log message as null-terminated string
 */
typedef void (*BD_LOG_FUNC)(const char *msg);

/**
 * Set (global) debug handler
 *
 * The function will receive all enabled log messages.
 *
 * @param handler function that will receive all enabled log and trace messages
 *
 */
void bd_set_debug_handler(BD_LOG_FUNC handler);

/**
 * Set (global) debug mask
 *
 * @param mask combination of flags from debug_mask_enum
 */
void bd_set_debug_mask(uint32_t mask);

/**
 * Get current (global) debug mask
 *
 * @return combination of flags from debug_mask_enum
 */
uint32_t bd_get_debug_mask(void);

#ifdef __cplusplus
}
#endif

#endif /* BD_LOG_CONTROL_H_ */

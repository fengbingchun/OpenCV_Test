/*
 * This file is part of libbluray
 * Copyright (C) 2010 fraxinas
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
 * \brief Disc metadata definitions
 */

#if !defined(_META_DATA_H_)
#define _META_DATA_H_

#include <stdint.h>

/** Thumbnail path and resolution */
typedef struct meta_thumbnail {
    char *           path;             /**< Path to thumbnail image (relative to disc root) */
    uint32_t         xres;             /**< Thumbnail width */
    uint32_t         yres;             /**< Thumbnail height */
} META_THUMBNAIL;

/** Title name */
typedef struct meta_title {
    uint32_t         title_number;     /**< Title number (from disc index) */
    char *           title_name;       /**< Title name */
} META_TITLE;

/** DL (Disc Library) metadata entry */
typedef struct meta_dl {
    char             language_code[4]; /**< Language used in this metadata entry */
    char *           filename;         /**< Source file (relative to disc root) */
    char *           di_name;          /**< Disc name */
    char *           di_alternative;   /**< Alternative name */
    uint8_t          di_num_sets;      /**< Number of discs in original volume or collection */
    uint8_t          di_set_number;    /**< Sequence order of the disc from an original collection */
    uint32_t         toc_count;        /**< Number of title entries */
    META_TITLE *     toc_entries;      /**< Title data */
    uint8_t          thumb_count;      /**< Number of thumbnails */
    META_THUMBNAIL * thumbnails;       /**< Thumbnail data */
} META_DL;

#endif // _META_DATA_H_


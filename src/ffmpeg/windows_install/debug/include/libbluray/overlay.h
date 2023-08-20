/*
 * This file is part of libbluray
 * Copyright (C) 2010-2017  Petri Hintukainen <phintuka@users.sourceforge.net>
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
 * \brief Graphics overlay events
 */

#ifndef BD_OVERLAY_H_
#define BD_OVERLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** Version number of the interface described in this file. */
#define BD_OVERLAY_INTERFACE_VERSION 2

/**
 * Overlay plane
 */
typedef enum {
    BD_OVERLAY_PG = 0,  /**< Presentation Graphics plane */
    BD_OVERLAY_IG = 1,  /**< Interactive Graphics plane (on top of PG plane) */
} bd_overlay_plane_e;

/*
 * Compressed YUV overlays
 */

/**
 * YUV overlay event type
 */
typedef enum {
    /* following events are executed immediately */
    BD_OVERLAY_INIT  = 0,    /**< Initialize overlay plane. Size and position of plane in x,y,w,h. */
    BD_OVERLAY_CLOSE = 1,    /**< Close overlay plane */

    /* following events can be processed immediately, but changes
     * should not be flushed to display before next FLUSH event
     */
    BD_OVERLAY_CLEAR = 2,    /**< Clear overlay plane */
    BD_OVERLAY_DRAW  = 3,    /**< Draw bitmap. Size and position within plane (x, y, w, h) and image (img, palette). */
    BD_OVERLAY_WIPE  = 4,    /**< Clear area. Size and position within plane (x, y, w, h). */
    BD_OVERLAY_HIDE  = 5,    /**< Overlay is empty and can be hidden */

    BD_OVERLAY_FLUSH = 6,    /**< All changes have been done, flush overlay to display at given pts */

} bd_overlay_cmd_e;

/**
 * Overlay palette entry
 *
 * Y, Cr and Cb have the same color matrix as the associated video stream.
 *
 * Entry 0xff is always transparent.
 *
 */
typedef struct bd_pg_palette_entry_s {
    uint8_t Y;      /**< Y component  (16...235) */
    uint8_t Cr;     /**< Cr component (16...240) */
    uint8_t Cb;     /**< Cb component (16...240) */
    uint8_t T;      /**< Transparency ( 0...255). 0 - transparent, 255 - opaque. */
} BD_PG_PALETTE_ENTRY;

/**
 * RLE element
 */
typedef struct bd_pg_rle_elem_s {
    uint16_t len;   /**< RLE run length */
    uint16_t color; /**< palette index */
} BD_PG_RLE_ELEM;

/**
 * YUV overlay event
 */
typedef struct bd_overlay_s {
    int64_t  pts;   /**< Timestamp, on video grid */
    uint8_t  plane; /**< Overlay plane (\ref bd_overlay_plane_e) */
    uint8_t  cmd;   /**< Overlay event type (\ref bd_overlay_cmd_e) */

    uint8_t  palette_update_flag; /**< Set if only overlay palette is changed */

    uint16_t x;     /**< top-left x coordinate */
    uint16_t y;     /**< top-left y coordinate */
    uint16_t w;     /**< region width */
    uint16_t h;     /**< region height */

    const BD_PG_PALETTE_ENTRY * palette; /**< overlay palette (256 entries) */
    const BD_PG_RLE_ELEM      * img;     /**< RLE-compressed overlay image */

} BD_OVERLAY;

/*
  RLE images are reference-counted. If application caches rle data for later use,
  it needs to use bd_refcnt_inc() and bd_refcnt_dec().
*/

const void *bd_refcnt_inc(const void *); /**< Hold reference-counted object. Return object or NULL on invalid object. */
void bd_refcnt_dec(const void *);        /**< Release reference-counted object */

#if 0
BD_OVERLAY *bd_overlay_copy(const BD_OVERLAY *src)
{
    BD_OVERLAY *ov = malloc(sizeof(*ov));
    memcpy(ov, src, sizeof(*ov));
    if (ov->palette) {
        ov->palette = malloc(256 * sizeof(BD_PG_PALETTE_ENTRY));
        memcpy((void*)ov->palette, src->palette, 256 * sizeof(BD_PG_PALETTE_ENTRY));
    }
    if (ov->img) {
        bd_refcnt_inc(ov->img);
    }
    return ov;
}

void bd_overlay_free(BD_OVERLAY **pov)
{
    if (pov && *pov) {
        BD_OVERLAY *ov = *pov;
        void *p = (void*)ov->palette;
        bd_refcnt_dec(ov->img);
        X_FREE(p);
        ov->palette = NULL;
        X_FREE(*pov);
    }
}
#endif

/**
 * ARGB overlay event type
 */
typedef enum {
    /* following events are executed immediately */
    BD_ARGB_OVERLAY_INIT  = 0,    /**< Initialize overlay plane. Size and position of plane are in x,y,w,h */
    BD_ARGB_OVERLAY_CLOSE = 1,    /**< Close overlay plane */

    /* following events can be processed immediately, but changes
     * should not be flushed to display before next FLUSH event
     */
    BD_ARGB_OVERLAY_DRAW  = 3,    /**< Draw ARGB image on plane */
    BD_ARGB_OVERLAY_FLUSH = 6,    /**< All changes have been done, flush overlay to display at given pts */
} bd_argb_overlay_cmd_e;

/**
 * ARGB overlay event
 */
typedef struct bd_argb_overlay_s {
    int64_t  pts;   /**< Event timestamp, on video grid */
    uint8_t  plane; /**< Overlay plane (\ref bd_overlay_plane_e) */
    uint8_t  cmd;   /**< Overlay event type (\ref bd_argb_overlay_cmd_e) */

    /* following fileds are used only when not using application-allocated
     * frame buffer
     */

    /* destination clip on the overlay plane */
    uint16_t x;     /**< top-left x coordinate */
    uint16_t y;     /**< top-left y coordinate */
    uint16_t w;     /**< region width */
    uint16_t h;     /**< region height */

    uint16_t stride;       /**< ARGB buffer stride */
    const uint32_t * argb; /**< ARGB image data, 'h' lines, line stride 'stride' pixels */

} BD_ARGB_OVERLAY;

/**
 * Application-allocated frame buffer for ARGB overlays
 *
 * When using application-allocated frame buffer DRAW events are
 * executed by libbluray.
 * Application needs to handle only OPEN/FLUSH/CLOSE events.
 *
 * DRAW events can still be used for optimizations.
 */
typedef struct bd_argb_buffer_s {
    /* optional lock / unlock functions
     *  - Set by application
     *  - Called when buffer is accessed or modified
     */
    void (*lock)  (struct bd_argb_buffer_s *); /**< Lock (or prepare) buffer for writing */
    void (*unlock)(struct bd_argb_buffer_s *); /**< Unlock buffer (write complete) */

    /* ARGB frame buffers
     * - Allocated by application (BD_ARGB_OVERLAY_INIT).
     * - Buffer can be freed after BD_ARGB_OVERLAY_CLOSE.
     * - buffer can be replaced in overlay callback or lock().
     */

    uint32_t *buf[4]; /**< [0] - PG plane, [1] - IG plane. [2], [3] reserved for stereoscopic overlay. */

    /* size of buffers
     * - Set by application
     * - If the buffer size is smaller than the size requested in BD_ARGB_OVERLAY_INIT,
     *   the buffer points only to the dirty area.
     */
    int width;   /**< overlay buffer width (pixels) */
    int height;  /**< overlay buffer height (pixels) */

    /** Dirty area of frame buffers
     * - Updated by library before lock() call.
     * - Reset after each BD_ARGB_OVERLAY_FLUSH.
     */
    struct {
        uint16_t x0; /**< top-left x coordinate */
        uint16_t y0; /**< top-left y coordinate */
        uint16_t x1; /**< bottom-down x coordinate  */
        uint16_t y1; /**< bottom-down y coordinate */
    } dirty[2]; /**< [0] - PG plane, [1] - IG plane */

} BD_ARGB_BUFFER;

#ifdef __cplusplus
}
#endif

#endif // BD_OVERLAY_H_

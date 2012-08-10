/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef OVERLAY_COMMON_H_
#define OVERLAY_COMMON_H_

//#include <hardware/overlay.h>
//#include "v4l2_utils.h"

#define NUM_BUFFERS_TO_BE_QUEUED_FOR_OPTIMAL_PERFORMANCE    2
#define NUM_OVERLAY_BUFFERS_REQUESTED  (2)
/* These values should come from Surface Flinger */
#define LCD_WIDTH 864
#define LCD_HEIGHT 480
#define TV_WIDTH 720
#define TV_HEIGHT 480
#define MAX_NUM_OVERLAYS 2
#define NUM_OVERLAY_BUFFERS_MAX NUM_OVERLAY_BUFFERS_REQUESTED

/** As the PV ARM Codecs are using only two output buffers, we can't have
* more than 2 buffers queued in the DSS queue
*/
#define NUM_BUFFERS_TO_BE_QUEUED_FOR_ARM_CODECS    2

/* Used in setAttributes */
#define CACHEABLE_BUFFERS 0x1
#define MAINTAIN_COHERENCY 0x2
#define OPTIMAL_QBUF_CNT    0x4

/* The following defines are used to set the maximum values supported
 * by the overlay.
 * 720p is the max resolution currently supported (1280x720)
 * */

#define MAX_OVERLAY_WIDTH_VAL (1280)
#define MAX_OVERLAY_HEIGHT_VAL (1280)
#define MAX_OVERLAY_RESOLUTION (1280 * 720)

#endif  // OVERLAY_COMMON_H_


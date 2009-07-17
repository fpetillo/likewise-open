/*
 * Copyright (c) Likewise Software.  All rights Reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Module Name:
 *
 *        common.h
 *
 * Abstract:
 *
 *        Common definitions (public header)
 *
 * Authors: Brian Koropoff (bkoropoff@likewisesoftware.com)
 *
 */
#ifndef __LWMSG_COMMON_H__
#define __LWMSG_COMMON_H__

/**
 * @file common.h Common definitions
 * @brief Common definitions
 */

/**
 * @defgroup public Public APIs
 *
 * These APIs are intended for direct consumption by client applications and
 * encompass the most common use cases.
 */

/**
 * @defgroup common Common definitions
 * @ingroup public
 * @brief Definitions of common types
 *
 * Defines various common types which are used through <tt>LWMsg</tt>.
 */

/* @{ */

/**
 * @brief Boolean type
 *
 * Represents a boolean true/false value
 */
typedef enum LWMsgBool
{
    /** False */
    LWMSG_FALSE = 0,
    /** True */
    LWMSG_TRUE = 1
} LWMsgBool;

/* @} */

#endif

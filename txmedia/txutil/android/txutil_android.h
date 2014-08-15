/*****************************************************************************
 * txutil_android.h
 *****************************************************************************
 *
 * copyright (c) 2013 Yu Yun Long <yuyunlong1987@163.com>
 *
 * This file is part of txPlayer.
 *
 * txPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * txPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with txPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __TXUTIL_ANDROID_H
#define __TXUTIL_ANDROID_H

#include "jnihelp.h"

#define JNI_CHECK_GOTO(condition__, env__, exception__, msg__, label__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                jniThrowException(env__, exception__, msg__); \
            } \
            goto label__; \
        } \
    }while(0)

#define JNI_CHECK_RET_VOID(condition__, env__, exception__, msg__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                jniThrowException(env__, exception__, msg__); \
            } \
            return; \
        } \
    }while(0)

#define JNI_CHECK_RET(condition__, env__, exception__, msg__, ret__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                jniThrowException(env__, exception__, msg__); \
            } \
            return ret__; \
        } \
    }while(0)

#endif

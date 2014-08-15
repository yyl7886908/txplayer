# copyright (c) 2014 Yu Yun Long <yuyunlong1987@163.com>
#
# This file is part of txPlayer.
#
# txPlayer is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# txPlayer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with txPlayer; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

LOCAL_PATH := $(call my-dir)

#--------------------
# C files
#--------------------
include $(CLEAR_VARS)
LOCAL_CFLAGS += -std=c99

LOCAL_SRC_FILES += android/loghelp.c

LOCAL_MODULE := txutil_c
include $(BUILD_STATIC_LIBRARY)


#--------------------
# CPP files: android
#--------------------
include $(CLEAR_VARS)
LOCAL_SRC_FILES += android/jnihelp.cpp

LOCAL_MODULE := txutil_android_cpp
include $(BUILD_STATIC_LIBRARY)


#--------------------
# so
#--------------------
include $(CLEAR_VARS)
LOCAL_LDLIBS += -llog

LOCAL_STATIC_LIBRARIES := txutil_c
LOCAL_WHOLE_STATIC_LIBRARIES := txutil_android_cpp

LOCAL_MODULE := txutil
include $(BUILD_SHARED_LIBRARY)


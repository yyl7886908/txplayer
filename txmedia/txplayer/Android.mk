#
# Copyright (c) 2014 Yu Yun Long <yuyunlong1987@163.com>
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
#$(warning playerplayerplayer)
include $(CLEAR_VARS)
LOCAL_CFLAGS += -std=c99
LOCAL_LDLIBS += -llog -landroid

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/..)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)

LOCAL_SRC_FILES += ff_cmdutils.c
LOCAL_SRC_FILES += ff_ffplay.c
LOCAL_SRC_FILES += txplayer.c

LOCAL_SRC_FILES += android/txplayer_android.c
LOCAL_SRC_FILES += android/txplayer_jni.c

LOCAL_SHARED_LIBRARIES := txffmpeg txutil txsdl txadk

LOCAL_MODULE := txplayer
include $(BUILD_SHARED_LIBRARY)

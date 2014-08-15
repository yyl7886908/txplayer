# copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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
#$(warning sdlsdlsd)
include $(CLEAR_VARS)
LOCAL_CFLAGS += -std=c99
LOCAL_LDLIBS += -llog -landroid

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/..)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/../txyuv/include)

LOCAL_SRC_FILES += txsdl_aout.c
LOCAL_SRC_FILES += txsdl_audio.c
LOCAL_SRC_FILES += txsdl_error.c
LOCAL_SRC_FILES += txsdl_mutex.c
LOCAL_SRC_FILES += txsdl_stdinc.c
LOCAL_SRC_FILES += txsdl_thread.c
LOCAL_SRC_FILES += txsdl_timer.c
LOCAL_SRC_FILES += txsdl_vout.c

#LOCAL_SRC_FILES += dummy/ijksdl_vout_dummy.c
LOCAL_SRC_FILES += dummy/txsdl_vout_dummy.c

#LOCAL_SRC_FILES += ffmpeg/ijksdl_vout_overlay_ffmpeg.c
LOCAL_SRC_FILES += ffmpeg/txsdl_vout_overlay_ffmpeg.c
LOCAL_SRC_FILES += ffmpeg/abi_all/image_convert.c

LOCAL_SRC_FILES += android/android_audiotrack.c
LOCAL_SRC_FILES += android/android_nativewindow.c
#LOCAL_SRC_FILES += android/ijksdl_android_jni.c
LOCAL_SRC_FILES += android/txsdl_android_jni.c
#LOCAL_SRC_FILES += android/ijksdl_aout_android_audiotrack.c
LOCAL_SRC_FILES += android/txsdl_aout_android_audiotrack.c
#LOCAL_SRC_FILES += android/ijksdl_vout_android_nativewindow.c
LOCAL_SRC_FILES += android/txsdl_vout_android_nativewindow.c
#LOCAL_SRC_FILES += android/ijksdl_vout_android_surface.c
LOCAL_SRC_FILES += android/txsdl_vout_android_surface.c

LOCAL_SHARED_LIBRARIES := txffmpeg txutil
LOCAL_STATIC_LIBRARIES := cpufeatures yuv_static

LOCAL_MODULE := txsdl
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)


LOCAL_PATH := $(call my-dir)
$(warning LOCAL_PATH: $(LOCAL_PATH))

### from ijkffmpeg
IJK_ANDROID_ROOT := $(realpath ../../../../ijk/android)
# $(warning IJK_ANDROID_ROOT: $(IJK_ANDROID_ROOT))

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
MY_APP_FFMPEG_OUTPUT_PATH := $(IJK_ANDROID_ROOT)/contrib/build/ffmpeg-armv7a/output
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
MY_APP_FFMPEG_OUTPUT_PATH := $(IJK_ANDROID_ROOT)/contrib/build/ffmpeg-arm64/output
endif

MY_APP_FFMPEG_INCLUDE_PATH := $(MY_APP_FFMPEG_OUTPUT_PATH)/include

$(warning MY_APP_FFMPEG_OUTPUT_PATH-1: $(MY_APP_FFMPEG_OUTPUT_PATH))

include $(call all-subdir-makefiles)

### end of ijkffmpeg


# include $(CLEAR_VARS)
# LOCAL_MODULE := ffmpeg
# LOCAL_SRC_FILES := prebuilt/libffmpeg.so

# include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

# LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)

# LOCAL_SHARED_LIBRARIES := libffmpeg # libcutil libstlport
LOCAL_STATIC_LIBRARIES += avformat avcodec swscale swresample avfilter avutil

LOCAL_CFLAGS += -D__STDC_CONSTANT_MACROS=1

LOCAL_MODULE    := videosurface

LOCAL_SRC_FILES := ../surface.cpp ../player.cpp ../util.cpp ../vdecode.cpp ../shader.cpp

LOCAL_LDLIBS    += -llog -landroid -lEGL -lGLESv2 -lc -lz

$(warning build-shared-lib: $(BUILD_SHARED_LIBRARY))
include $(BUILD_SHARED_LIBRARY)


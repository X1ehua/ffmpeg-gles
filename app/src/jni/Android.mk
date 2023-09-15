LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES := prebuilt/libffmpeg.so

include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_MODULE    := videosurface
LOCAL_SRC_FILES := surface.cpp player.cpp util.cpp vdecode.cpp shader.cpp

LOCAL_LDLIBS    += -llog
LOCAL_LDLIBS    += -landroid # for native windows

LOCAL_LDLIBS 	+= -lEGL
LOCAL_LDLIBS 	+= -lGLESv2
LOCAL_LDLIBS 	+= -lc

LOCAL_SHARED_LIBRARIES := libffmpeg
# LOCAL_SHARED_LIBRARIES += libcutil
# LOCAL_SHARED_LIBRARIES += libstlport

LOCAL_CFLAGS += -D__STDC_CONSTANT_MACROS=1

include $(BUILD_SHARED_LIBRARY)

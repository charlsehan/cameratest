LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := cameratest

#LOCAL_PROPRIETARY_MODULE := true
# PROPRIETARY, outputs will goto /system/vendor

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := Main.cpp \
                   CmdMap.cpp \
                   Cmd_Exit.cpp \
                   Cmd_Test.cpp \
                   Utils.cpp \
                   vtouch/ImageProcessor.cpp \
                   vtouch/OpenCVProcessor.cpp \
                   vtouch/VirtualTouch.cpp

LOCAL_INIT_RC := camtest.rc

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += \
    system/media/private/camera/include \
    frameworks/native/include/media/openmax \
    external/jpeg \
    system/core/opencv/include

LOCAL_SHARED_LIBRARIES:= \
    libui \
    liblog \
    libutils \
    libbinder \
    libcutils \
    libmedia \
    libmediautils \
    libcamera_client \
    libgui \
    libhardware \
    libsync \
    libcamera_metadata \
    libjpeg \
    libmemunreachable \
    libcamera_client \
    libmtkcam_fwkutils \
    libopencv_java3

LOCAL_CPPFLAGS += -fexceptions -Wno-non-virtual-dtor

include $(BUILD_EXECUTABLE)


LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := dumpstate.cpp utils.cpp

LOCAL_MODULE := dumpstate

LOCAL_SHARED_LIBRARIES := libcutils libdebuggerd_client liblog libselinux libbase
# ZipArchive support, the order matters here to get all symbols.
LOCAL_STATIC_LIBRARIES := libziparchive libz libcrypto_static
LOCAL_HAL_STATIC_LIBRARIES := libdumpstate
LOCAL_CFLAGS += -Wall -Werror -Wno-unused-parameter
LOCAL_INIT_RC := dumpstate.rc

include $(BUILD_EXECUTABLE)

LOCAL_PATH:= $(call my-dir)

# ================#
# Common settings #
# ================#
# ZipArchive support, the order matters here to get all symbols.
COMMON_ZIP_LIBRARIES := libziparchive libz libcrypto

# TODO: ideally the tests should depend on a shared dumpstate library, but currently libdumpstate
# is used to define the device-specific HAL library. Instead, both dumpstate and dumpstate_test
# shares a lot of common settings
COMMON_LOCAL_CFLAGS := \
       -Wall -Werror -Wno-missing-field-initializers -Wno-unused-variable -Wunused-parameter
COMMON_SRC_FILES := \
        DumpstateInternal.cpp \
        utils.cpp
COMMON_SHARED_LIBRARIES := \
        android.hardware.dumpstate@1.0 \
        android.hidl.manager@1.0 \
        libhidlbase \
        libbase \
        libbinder \
        libcutils \
        libdebuggerd_client \
        libdumpstateaidl \
        libdumpstateutil \
        liblog \
        libselinux \
        libutils \
        $(COMMON_ZIP_LIBRARIES)

# ====================#
# libdumpstateutil #
# ====================#
include $(CLEAR_VARS)

LOCAL_MODULE := libdumpstateutil

LOCAL_CFLAGS := $(COMMON_LOCAL_CFLAGS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_SRC_FILES := \
        DumpstateInternal.cpp \
        DumpstateUtil.cpp
LOCAL_SHARED_LIBRARIES := \
        libbase \
        liblog \

include $(BUILD_SHARED_LIBRARY)

# ====================#
# libdumpstateheaders #
# ====================#
# TODO: this module is necessary so the device-specific libdumpstate implementations do not
# need to add any other dependency (like libbase). Should go away once dumpstate HAL changes.
include $(CLEAR_VARS)

LOCAL_EXPORT_C_INCLUDE_DIRS = $(LOCAL_PATH)
LOCAL_MODULE := libdumpstateheaders
LOCAL_EXPORT_SHARED_LIBRARY_HEADERS := \
        $(COMMON_SHARED_LIBRARIES)
LOCAL_EXPORT_STATIC_LIBRARY_HEADERS := \
        $(COMMON_STATIC_LIBRARIES)
# Soong requires that whats is on LOCAL_EXPORTED_ is also on LOCAL_
LOCAL_SHARED_LIBRARIES := $(LOCAL_EXPORT_SHARED_LIBRARY_HEADERS)
LOCAL_STATIC_LIBRARIES := $(LOCAL_EXPORT_STATIC_LIBRARY_HEADERS)

include $(BUILD_STATIC_LIBRARY)

# ================ #
# libdumpstateaidl #
# =================#
include $(CLEAR_VARS)

LOCAL_MODULE := libdumpstateaidl

LOCAL_CFLAGS := $(COMMON_LOCAL_CFLAGS)

LOCAL_SHARED_LIBRARIES := \
        libbinder \
        libutils
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/binder
LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/binder
LOCAL_C_INCLUDES := $(LOCAL_PATH)/binder
LOCAL_SRC_FILES := \
        binder/android/os/IDumpstate.aidl \
        binder/android/os/IDumpstateListener.aidl \
        binder/android/os/IDumpstateToken.aidl

include $(BUILD_SHARED_LIBRARY)

# ==========#
# dumpstate #
# ==========#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(COMMON_SRC_FILES) \
        DumpstateService.cpp \
        dumpstate.cpp

LOCAL_MODULE := dumpstate

LOCAL_SHARED_LIBRARIES := $(COMMON_SHARED_LIBRARIES)

LOCAL_STATIC_LIBRARIES := $(COMMON_STATIC_LIBRARIES)

LOCAL_CFLAGS += $(COMMON_LOCAL_CFLAGS)

LOCAL_INIT_RC := dumpstate.rc

include $(BUILD_EXECUTABLE)

# ===============#
# dumpstate_test #
# ===============#
include $(CLEAR_VARS)

LOCAL_MODULE := dumpstate_test

LOCAL_MODULE_TAGS := tests

LOCAL_CFLAGS := $(COMMON_LOCAL_CFLAGS)

LOCAL_SRC_FILES := $(COMMON_SRC_FILES) \
        DumpstateService.cpp \
        tests/dumpstate_test.cpp

LOCAL_STATIC_LIBRARIES := $(COMMON_STATIC_LIBRARIES) \
        libgmock

LOCAL_SHARED_LIBRARIES := $(COMMON_SHARED_LIBRARIES)

include $(BUILD_NATIVE_TEST)

# =======================#
# dumpstate_test_fixture #
# =======================#
include $(CLEAR_VARS)

LOCAL_MODULE := dumpstate_test_fixture

LOCAL_MODULE_TAGS := tests

LOCAL_CFLAGS := $(COMMON_LOCAL_CFLAGS)

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

LOCAL_SRC_FILES := \
        tests/dumpstate_test_fixture.cpp

LOCAL_MODULE_CLASS := NATIVE_TESTS

dumpstate_tests_intermediates := $(local-intermediates-dir)/DATA
dumpstate_tests_subpath_from_data := nativetest/dumpstate_test_fixture
dumpstate_tests_root_in_device := /data/$(dumpstate_tests_subpath_from_data)
dumpstate_tests_root_for_test_zip := $(dumpstate_tests_intermediates)/$(dumpstate_tests_subpath_from_data)
testdata_files := $(call find-subdir-files, testdata/*)

# Copy test data files to intermediates/DATA for use with LOCAL_PICKUP_FILES
GEN := $(addprefix $(dumpstate_tests_root_for_test_zip)/, $(testdata_files))
$(GEN): PRIVATE_PATH := $(LOCAL_PATH)
$(GEN): PRIVATE_CUSTOM_TOOL = cp $< $@
$(GEN): $(dumpstate_tests_root_for_test_zip)/testdata/% : $(LOCAL_PATH)/testdata/%
	$(transform-generated-source)
LOCAL_GENERATED_SOURCES += $(GEN)

# Copy test data files again to $OUT/data so the tests can be run with adb sync
# TODO: the build system should do this automatically
GEN := $(addprefix $(TARGET_OUT_DATA)/$(dumpstate_tests_subpath_from_data)/, $(testdata_files))
$(GEN): PRIVATE_PATH := $(LOCAL_PATH)
$(GEN): PRIVATE_CUSTOM_TOOL = cp $< $@
$(GEN): $(TARGET_OUT_DATA)/$(dumpstate_tests_subpath_from_data)/testdata/% : $(LOCAL_PATH)/testdata/%
	$(transform-generated-source)
LOCAL_GENERATED_SOURCES += $(GEN)

LOCAL_PICKUP_FILES := $(dumpstate_tests_intermediates)

include $(BUILD_NATIVE_TEST)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE     := lualexer
LOCAL_SRC_FILES  := lualexer.c lexer.c
LOCAL_STATIC_LIBRARIES := luajava

include $(BUILD_SHARED_LIBRARY)

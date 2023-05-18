LOCAL_PATH := $(call my-dir)/..

#LOCAL_ADDRESS_SANITIZER:=true
#USE_BUILTIN_LUA:=true

include $(CLEAR_VARS)
LOCAL_MODULE := Freetype
LOCAL_SRC_FILES := deps/$(APP_ABI)/Freetype/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Iconv
LOCAL_SRC_FILES := deps/$(APP_ABI)/Iconv/libiconv.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcharset
LOCAL_SRC_FILES := deps/$(APP_ABI)/Iconv/libcharset.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Irrlicht
LOCAL_SRC_FILES := deps/$(APP_ABI)/Irrlicht/libIrrlichtMt.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Irrlicht-libpng
LOCAL_SRC_FILES := deps/$(APP_ABI)/Irrlicht/libpng.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Irrlicht-libjpeg
LOCAL_SRC_FILES := deps/$(APP_ABI)/Irrlicht/libjpeg.a
include $(PREBUILT_STATIC_LIBRARY)

ifndef USE_BUILTIN_LUA

include $(CLEAR_VARS)
LOCAL_MODULE := LuaJIT
LOCAL_SRC_FILES := deps/$(APP_ABI)/LuaJIT/libluajit.a
include $(PREBUILT_STATIC_LIBRARY)

endif

include $(CLEAR_VARS)
LOCAL_MODULE := OpenAL
LOCAL_SRC_FILES := deps/$(APP_ABI)/OpenAL-Soft/libopenal.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Vorbis
LOCAL_SRC_FILES := deps/$(APP_ABI)/Vorbis/libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvorbisfile
LOCAL_SRC_FILES := deps/$(APP_ABI)/Vorbis/libvorbisfile.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libogg
LOCAL_SRC_FILES := deps/$(APP_ABI)/Vorbis/libogg.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Zstd
LOCAL_SRC_FILES := deps/$(APP_ABI)/Zstd/libzstd.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Minetest

LOCAL_CFLAGS += \
	-DJSONCPP_NO_LOCALE_SUPPORT     \
	-DHAVE_TOUCHSCREENGUI           \
	-DUSE_SOUND=1                   \
	-DUSE_GETTEXT=0                 \
	-DVERSION_MAJOR=${versionMajor} \
	-DVERSION_MINOR=${versionMinor} \
	-DVERSION_PATCH=${versionPatch} \
	-DVERSION_EXTRA=${versionExtra} \
	-DDEVELOPMENT_BUILD=${developmentBuild} \
	$(GPROF_DEF)

ifdef USE_BUILTIN_LUA
	LOCAL_CFLAGS += -DUSE_LUAJIT=0
else
	LOCAL_CFLAGS += -DUSE_LUAJIT=1
endif

ifdef NDEBUG
	LOCAL_CFLAGS += -DNDEBUG=1
endif

ifdef GPROF
	GPROF_DEF := -DGPROF
	PROFILER_LIBS := android-ndk-profiler
	LOCAL_CFLAGS += -pg
endif

LOCAL_C_INCLUDES := \
	../../src                                    \
	../../src/script                             \
	../../lib/gmp                                \
	../../lib/jsoncpp                            \
	deps/$(APP_ABI)/Freetype/include/freetype2         \
	deps/$(APP_ABI)/Irrlicht/include                   \
	deps/$(APP_ABI)/Iconv/include                      \
	deps/$(APP_ABI)/OpenAL-Soft/include                \
	deps/$(APP_ABI)/Vorbis/include                     \
	deps/$(APP_ABI)/Zstd/include

ifdef USE_BUILTIN_LUA
	LOCAL_C_INCLUDES += \
		../../lib/lua/src
else
	LOCAL_C_INCLUDES += deps/$(APP_ABI)/LuaJIT/include
endif

LOCAL_SRC_FILES := \
	$(wildcard ../../src/client/*.cpp)           \
	$(wildcard ../../src/client/*/*.cpp)         \
	$(wildcard ../../src/content/*.cpp)          \
	$(wildcard ../../src/database/*.cpp)          \
	$(wildcard ../../src/gui/*.cpp)              \
	$(wildcard ../../src/irrlicht_changes/*.cpp) \
	$(wildcard ../../src/network/*.cpp)          \
	$(wildcard ../../src/script/*.cpp)           \
	$(wildcard ../../src/script/*/*.cpp)         \
	$(wildcard ../../src/server/*.cpp)           \
	$(wildcard ../../src/threading/*.cpp)        \
	$(wildcard ../../src/util/*.c)               \
	$(wildcard ../../src/util/*.cpp)             \
	$(wildcard ../../src/*.cpp)

# Built-in Lua
ifdef USE_BUILTIN_LUA
	LOCAL_SRC_FILES += \
		../../lib/lua/src/lapi.c \
		../../lib/lua/src/lauxlib.c \
		../../lib/lua/src/lbaselib.c \
		../../lib/lua/src/lcode.c \
		../../lib/lua/src/ldblib.c \
		../../lib/lua/src/ldebug.c \
		../../lib/lua/src/ldo.c \
		../../lib/lua/src/ldump.c \
		../../lib/lua/src/lfunc.c \
		../../lib/lua/src/lgc.c \
		../../lib/lua/src/linit.c \
		../../lib/lua/src/liolib.c \
		../../lib/lua/src/llex.c \
		../../lib/lua/src/lmathlib.c \
		../../lib/lua/src/lmem.c \
		../../lib/lua/src/loadlib.c \
		../../lib/lua/src/lobject.c \
		../../lib/lua/src/lopcodes.c \
		../../lib/lua/src/loslib.c \
		../../lib/lua/src/lparser.c \
		../../lib/lua/src/lstate.c \
		../../lib/lua/src/lstring.c \
		../../lib/lua/src/lstrlib.c \
		../../lib/lua/src/ltable.c \
		../../lib/lua/src/ltablib.c \
		../../lib/lua/src/ltm.c \
		../../lib/lua/src/lundump.c \
		../../lib/lua/src/lvm.c \
		../../lib/lua/src/lzio.c
endif

# GMP
LOCAL_SRC_FILES += ../../lib/gmp/mini-gmp.c

# JSONCPP
LOCAL_SRC_FILES += ../../lib/jsoncpp/jsoncpp.cpp

LOCAL_STATIC_LIBRARIES += \
	Freetype \
	Iconv libcharset \
	Irrlicht Irrlicht-libpng Irrlicht-libjpeg \
	OpenAL \
	Vorbis libvorbisfile libogg \
	Zstd
ifndef USE_BUILTIN_LUA
	LOCAL_STATIC_LIBRARIES += LuaJIT
endif
LOCAL_STATIC_LIBRARIES += android_native_app_glue $(PROFILER_LIBS)

LOCAL_LDLIBS := -lEGL -lGLESv1_CM -lGLESv2 -landroid -lOpenSLES -lz

include $(BUILD_SHARED_LIBRARY)

ifdef GPROF
$(call import-module,android-ndk-profiler)
endif
$(call import-module,android/native_app_glue)

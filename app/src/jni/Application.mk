
# APP_OPTIM := release
APP_OPTIM := debug
# APP_ABI := arm64-v8a
APP_ABI := armeabi-v7a
APP_PIE := false
APP_PLATFORM := android-23
NDK_TOOLCHAIN_VERSION=4.9

APP_STL := c++_static

APP_CFLAGS := -O0 -Wall -pipe \
    -ffast-math \
    -fstrict-aliasing -Werror=strict-aliasing \
    -Wno-psabi -Wa,--noexecstack \
    -Wno-deprecated-declarations \
    -DANDROID
#   -DANDROID -DNDEBUG

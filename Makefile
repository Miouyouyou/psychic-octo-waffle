OBJS = base_gl.o file.o myy.o
CC = gcc
CFLAGS = -march=native -g3 -O0 -fPIC
INCLUDE_DIRS = -I.
LDFLAGS = -lGLESv2
CCC = $(CC) $(CFLAGS) $(INCLUDE_DIRS)
LIBRARY = libmyy.so

.PHONY: all
all: x11 $(LIBRARY)

$(LIBRARY): $(OBJS)
	$(CCC) --shared -o $(LIBRARY) $(OBJS) $(LDFLAGS)

myy.o: myy.c
	$(CCC) -c myy.c

base_gl.o: helpers/base_gl.c
	$(CCC) -c helpers/base_gl.c

file.o: helpers/file.c
	$(CCC) -c helpers/file.c

.PHONY: x11
x11:
	$(MAKE) -C X11 all
	cp X11/Program ./

.PHONY: clean
clean:
	$(RM) *.{o,so} $(LIBRARY)
	$(MAKE) -C X11 clean

.PHONY: distclean
distclean: clean
	$(RM) Program
	$(RM) *~

ANDROID_CFLAGS = -fPIC -D__ANDROID__ -DANDROID -O3 -mthumb -mthumb-interwork -fuse-ld=gold -mfloat-abi=softfp -std=c11 -nostdlib
ANDROID_BASE_DIR = $(ANDROID_NDK_HOME)/platforms/android-15/arch-arm/usr
ANDROID_CC = armv7a-hardfloat-linux-gnueabi-gcc
ANDROID_CCC = $(ANDROID_CC) $(ANDROID_CFLAGS) -I$(ANDROID_BASE_DIR)/include -I.
ANDROID_LDFLAGS = -Wl,-soname=libmain.so,--dynamic-linker=/system/bin/linker,--hash-style=sysv -L$(ANDROID_BASE_DIR)/lib -lEGL -lGLESv2 -llog -landroid -lc
ANDROID_OBJS = android_native_app_glue.o android_dummy_main.o
ANDROID_APK_PATH = ./android_apk
ANDROID_APK_LIB_PATH = $(ANDROID_APK_PATH)/app/src/main/jniLibs

android_native_app_glue.o: android_native_app_glue.c
	$(CCC) -c android_native_app_glue.c

android_dummy_main.o: android_dummy_main.c
	$(CCC) -c android_dummy_main.c

.PHONY: android
android: CCC = $(ANDROID_CCC)
android: OBJS += $(ANDROID_OBJS)
android: $(OBJS) $(ANDROID_OBJS)
	$(ANDROID_CCC) --shared -o libmain.so $(OBJS) $(ANDROID_LDFLAGS)
	cp libmain.so $(ANDROID_APK_LIB_PATH)/armeabi/
	cp libmain.so $(ANDROID_APK_LIB_PATH)/armeabi-v7a/
	adb shell mkdir /sdcard/OpenGL
	adb push tex /sdcard/OpenGL/tex
	adb push shaders /sdcard/OpenGL/shaders
	$(MAKE) -C $(ANDROID_APK_PATH) install

# (Recursively) finds every file in a folder with given mask (super useful)
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

C=gcc
CC=g++
AR=ar -rc
BUILD_DIR ?= build
BUILD_DIRS ?= build build/server/server/Main build/client/client/Main build/glfw/vendor/glfw/src build/glfw/vendor/glfw/deps build/client/vendor/glad

TARGET_CLIENT_EXEC ?= client/terraluna_client.exe
TARGET_SERVER_EXEC ?= server/terraluna_server.exe
TARGET_GLFW_LIB ?= glfw/glfw.a



LINUX_CLIENT_CCFLAGS := -Ivendor/glfw/include -Ivendor/glad
LINUX_CLIENT_CFLAGS := -Ivendor/glfw/include -Ivendor/glad
LINUX_CLIENT_LDFLAGS := $(BUILD_DIR)/glfw/glfw.a -lGL -lm -lX11 -lpthread -lXi -lXrandr -ldl

CLIENT_SRCS := $(call rwildcard,client,*.cc) vendor/glad/glad.c
CLIENT_OBJS := $(CLIENT_SRCS:%=$(BUILD_DIR)/client/%.o)
CLIENT_CCFLAGS ?= -Ofast -s -std=c++17 $(LINUX_CLIENT_CCFLAGS)
CLIENT_CFLAGS ?= -std=c11 -Ofast -s -Iclient/include $(LINUX_CLIENT_CFLAGS)
CLIENT_LDFLAGS ?= -static-libgcc -static-libstdc++ $(LINUX_CLIENT_LDFLAGS)



SERVER_SRCS := $(call rwildcard,server,*.cc)
SERVER_OBJS := $(SERVER_SRCS:%=$(BUILD_DIR)/server/%.o)
SERVER_CCFLAGS ?= -Ofast -s -std=c++17
SERVER_LDFLAGS ?= -static-libgcc -static-libstdc++



# Sorry for that long source list
LINUX_GLFW_SRCS := $(wildcard vendor/glfw/src/*linux*.c) $(wildcard vendor/glfw/src/*posix*.c) $(wildcard vendor/glfw/src/*x11*.c) $(wildcard vendor/glfw/src/*xkb*.c) vendor/glfw/src/init.c vendor/glfw/src/input.c vendor/glfw/src/monitor.c vendor/glfw/src/vulkan.c vendor/glfw/src/window.c vendor/glfw/src/context.c vendor/glfw/src/glx_context.c vendor/glfw/src/egl_context.c vendor/glfw/src/osmesa_context.c vendor/glfw/src/linux_joystick.c $(wildcard vendor/glfw/deps/*.c)
WINDOWS_GLFW_SRCS := $(wildcard vendor/glfw/src/*win32*.c) vendor/glfw/src/init.c vendor/glfw/src/input.c vendor/glfw/src/monitor.c vendor/glfw/src/vulkan.c vendor/glfw/src/window.c vendor/glfw/src/context.c vendor/glfw/src/wgl_context.c vendor/glfw/src/egl_context.c vendor/glfw/src/osmesa_context.c vendor/glfw/src/null_joystick.c $(wildcardvendor/glfw/deps/*.c) 

LINUX_GLFW_INCLUDES := -Ivendor/glfw/src -Ivendor/glfw/include -Ivendor/glfw/deps -Ivendor/glfw/deps/glad

# This is platform dependent aswell
GLFW_SRCS := $(LINUX_GLFW_SRCS)
GLFW_INCLUDES := $(LINUX_GLFW_INCLUDES)

GLFW_OBJS := $(GLFW_SRCS:%=$(BUILD_DIR)/glfw/%.o)
GLFW_CFLAGS ?= -static -static-libgcc -Ofast -s -std=c11 -D_GLFW_X11=1 $(GLFW_INCLUDES)



# Linking client
$(BUILD_DIR)/$(TARGET_CLIENT_EXEC): $(BUILD_DIRS) $(BUILD_DIR)/$(TARGET_GLFW_LIB) $(CLIENT_OBJS)
	@echo Linking client
	@$(CC) $(CLIENT_OBJS) $(CLIENT_LDFLAGS) -o $@

# Linking server
$(BUILD_DIR)/$(TARGET_SERVER_EXEC): $(BUILD_DIRS) $(SERVER_OBJS)
	@echo Linking server
	@$(CC) $(SERVER_OBJS) $(SERVER_LDFLAGS) -o $@

# Packing GLFW
$(BUILD_DIR)/$(TARGET_GLFW_LIB): $(BUILD_DIRS) $(GLFW_OBJS)
	@echo Packing GLFW
	@$(AR) $@ $(GLFW_OBJS)



# Client sources
$(BUILD_DIR)/client/%.cc.o: %.cc
	@echo Building $<
	@$(CC) $(CLIENT_CCFLAGS) -c $< -o $@

$(BUILD_DIR)/client/%.c.o: %.c
	@echo Building $<
	@$(C) $(CLIENT_CFLAGS) -c $< -o $@


# Server sources
$(BUILD_DIR)/server/%.cc.o: %.cc
	@echo Building $<
	@$(CC) $(SERVER_CCFLAGS) -c $< -o $@


# GLFW sources
$(BUILD_DIR)/glfw/%.c.o: %.c
	@echo Building $<
	@$(C) $(GLFW_CFLAGS) -c $< -o $@



.PHONY: clean
clean:
	@echo Cleaning
	@rm -r $(BUILD_DIR)

$(BUILD_DIRS):
	@echo Making directory structure
	@mkdir -p $(BUILD_DIRS)

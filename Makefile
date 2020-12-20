# (Recursively) finds every file in a folder with given mask (super useful)
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

C=gcc
CC=g++
AR=ar -rc
BUILD_DIR ?= build

SERVER_BUILD_DIRS ?= $(BUILD_DIR)/server/server/Main
CLIENT_BUILD_DIRS ?= $(BUILD_DIR)/client/client/Main \
						$(BUILD_DIR)/client/client/Graphics/Shaders \
						$(BUILD_DIR)/client/client/Misc/Maths

GLFW_BUILD_DIRS ?= $(BUILD_DIR)/glfw/vendor/glfw/src \
					$(BUILD_DIR)/glfw/vendor/glfw/deps \


BUILD_DIRS ?= $(BUILD_DIR) \
				$(CLIENT_BUILD_DIRS) \
				$(SERVER_BUILD_DIRS) \
				$(GLFW_BUILD_DIRS) \
				$(BUILD_DIR)/client/vendor/glad



TARGET_CLIENT_EXEC ?= client/terraluna_client.exe
TARGET_SERVER_EXEC ?= server/terraluna_server.exe
TARGET_GLFW_LIB ?= glfw/glfw.a



LINUX_CLIENT_LDFLAGS := -Bstatic $(BUILD_DIR)/glfw/glfw.a -lm -lpthread -ldl -Bdynamic -lGL -lXi -lX11 -lXrandr

CLIENT_SHARED_INCLUDES := -Ivendor/glfw/include -Ivendor/glad -Ivendor/glm -Iclient
CLIENT_SRCS := $(call rwildcard,client,*.cc) vendor/glad/glad.c
CLIENT_OBJS := $(CLIENT_SRCS:%=$(BUILD_DIR)/client/%.o)
CLIENT_CCFLAGS ?= -Ofast -s -std=c++17 $(CLIENT_SHARED_INCLUDES) $(LINUX_CLIENT_CCFLAGS)
CLIENT_CFLAGS ?= -static-libgcc -std=c11 -Ofast -s $(CLIENT_SHARED_INCLUDES) $(LINUX_CLIENT_CFLAGS)
CLIENT_LDFLAGS ?= -static-libgcc -static-libstdc++ $(LINUX_CLIENT_LDFLAGS)



SERVER_SRCS := $(call rwildcard,server,*.cc)
SERVER_OBJS := $(SERVER_SRCS:%=$(BUILD_DIR)/server/%.o)
SERVER_CCFLAGS ?= -Ofast -s -std=c++17
SERVER_LDFLAGS ?= -static-libgcc -static-libstdc++



LINUX_GLFW_SRCS := $(wildcard vendor/glfw/src/*linux*.c) $(wildcard vendor/glfw/src/*posix*.c) $(wildcard vendor/glfw/src/*x11*.c) $(wildcard vendor/glfw/src/*xkb*.c) vendor/glfw/src/init.c vendor/glfw/src/input.c vendor/glfw/src/monitor.c vendor/glfw/src/vulkan.c vendor/glfw/src/window.c vendor/glfw/src/context.c vendor/glfw/src/glx_context.c vendor/glfw/src/egl_context.c vendor/glfw/src/osmesa_context.c vendor/glfw/src/linux_joystick.c $(wildcard vendor/glfw/deps/*.c)
WINDOWS_GLFW_SRCS := $(wildcard vendor/glfw/src/*win32*.c) vendor/glfw/src/init.c vendor/glfw/src/input.c vendor/glfw/src/monitor.c vendor/glfw/src/vulkan.c vendor/glfw/src/window.c vendor/glfw/src/context.c vendor/glfw/src/wgl_context.c vendor/glfw/src/egl_context.c vendor/glfw/src/osmesa_context.c vendor/glfw/src/null_joystick.c $(wildcardvendor/glfw/deps/*.c) 

GLFW_SRCS := $(LINUX_GLFW_SRCS)
GLFW_OBJS := $(GLFW_SRCS:%=$(BUILD_DIR)/glfw/%.o)
GLFW_INCLUDES := -Ivendor/glfw/src -Ivendor/glfw/include -Ivendor/glfw/deps -Ivendor/glfw/deps/glad -Ivendor/glad
GLFW_CFLAGS ?= -static -static-libgcc -Ofast -s -std=c11 -D_GLFW_X11=1 $(GLFW_INCLUDES)


# No progress indication here because the echo would break the check if file is outdated or not and still get relinked/repacked
# Linking client
$(BUILD_DIR)/$(TARGET_CLIENT_EXEC): $(BUILD_DIRS) $(BUILD_DIR)/$(TARGET_GLFW_LIB) $(CLIENT_OBJS)
	@$(CC) $(CLIENT_OBJS) $(CLIENT_LDFLAGS) -o $@

# Linking server
$(BUILD_DIR)/$(TARGET_SERVER_EXEC): $(BUILD_DIRS) $(SERVER_OBJS)
	@$(CC) $(SERVER_OBJS) $(SERVER_LDFLAGS) -o $@

# Packing GLFW
$(BUILD_DIR)/$(TARGET_GLFW_LIB): $(BUILD_DIRS) $(GLFW_OBJS)
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



.PHONY: clean makeclient runclient
clean:
	@rm -r $(BUILD_DIR)

$(BUILD_DIRS):
#	$(subst /,\\,$(BUILD_DIRS))
	@mkdir -p $(BUILD_DIRS)

makeclient: $(BUILD_DIR)/$(TARGET_CLIENT_EXEC)
runclient: $(BUILD_DIR)/$(TARGET_CLIENT_EXEC)
	@$(BUILD_DIR)/$(TARGET_CLIENT_EXEC)

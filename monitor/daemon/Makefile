######################################################
# Build configurations

include $(WORK_DIR)/lib_config.mk

######################################################
# Module library

MODULE_LIBRARY = monitord

MODULE_SRCS = $(wildcard source/*.c)

MODULE_OBJS = $(patsubst %.c,%.o,$(patsubst %.s,%.o,$(patsubst %.cpp,%.o,$(MODULE_SRCS))))

######################################################
# Header files

PUBLIC_HEADER_FILES =

######################################################
# Module flags

MODULE_CFLAGS := -I$(TOPDIR)/OSAL/include
MODULE_CFLAGS += -I$(TOPDIR)/monitor/libmonitor/include

######################################################
# Unittest setting

UNITTEST_SRCS =

UNITTEST_OBJS =

######################################################
# Release setting

# released source files
RELEASE_SRCS =

# released header files
RELEASE_HEADER_FILES =

# released library
RELEASE_LIBRARY = MODULE_LIBRARY

# source files to build released library
RELEASE_LIB_SRCS = MODULE_SRCS

# object files to build released library
RELEASE_LIB_OBJS = MODULE_OBJS

######################################################
# Common dependency process

include $(TOPDIR)/build/Scripts/mk_default.in

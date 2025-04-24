#*******************************************************************
# Terms of Use and Disclaimer
# Copyright 2011-2040 Sungkil Lee
#
# This software is owned by Sungkil Lee.
# You may use and modify this software for non-commercial and commercial software,
# but re-distribution of this software is strictly prohibited.
#*******************************************************************

# per-project variable definitions
# m64 (x64) or m32 (x86)
C		:= gcc
CC		:= g++
ARCH	:= m64

ifneq ($(OS), Windows_NT)
	C_SRC 	:= $(shell find * -type f -name "*.c")
	CC_SRC	:= $(shell find * -type f -name "*.cpp")
else ifneq ($(MSYSTEM),)
	C_SRC 	:= $(shell find * -type f -name "*.c")
	CC_SRC	:= $(shell find * -type f -name "*.cpp")
	# override gcc to clang
	ifeq ($(MSYSTEM),CLANG64)
		C	:= clang
		CC	:= clang++
	endif
else
	CWD		:= $(addsuffix \,$(shell cd))
	C_SRC 	:= $(subst \,/,$(subst $(CWD),,$(shell dir /s/b *.c)))
	CC_SRC	:= $(subst \,/,$(subst $(CWD),,$(shell dir /s/b *.cpp)))
endif

# name derived from vc project; so, don't delete vcxproj even in Linux
NAME := $(subst .vcxproj,,$(notdir $(wildcard *.vcxproj)))

# directories and header dependency
BIN := ../bin
INC := -I. -Igl -Igles
OBJ := .obj

#**************************************
# nearly fixed compiler flags/objects
C_FLAGS  := -c -$(ARCH) -Wall $(INC)
CC_FLAGS := $(C_FLAGS) -std=c++17
C_OBJS   := $(addprefix $(OBJ)/,$(C_SRC:.c=.o))
CC_OBJS  := $(addprefix $(OBJ)/,$(CC_SRC:.cpp=.o))

#**************************************
# os-dependent configuration
# Ubuntu/Linux
ifneq ($(OS), Windows_NT)
	TARGET = $(addsuffix .out,$(BIN)/$(NAME))
	# not glfw3 in Ubuntu/Linux
	LD_FLAGS := -lglfw
	MK_INT_DIR = @mkdir -p $(@D)
	RM_INT_DIR = @rm -rf $(OBJ)
	RM_TARGET = @rm -rf $(TARGET)
# MinGW	
else
	TARGET = $(addsuffix .exe,$(BIN)/$(NAME))
	# not glfw in MinGW; gdi32 should be linked
	LD_FLAGS := -static -lglfw3 -lgdi32
	ifeq ($(MSYSTEM), UCRT64)
		LD_FLAGS := $(LD_FLAGS) -lucrt
	endif
	MK_INT_DIR = @bash -c "mkdir -p $(@D)"
	RM_INT_DIR = @bash -c "rm -rf $(OBJ)"
	RM_TARGET = @bash -c "rm -rf $(TARGET)"
endif

#**************************************
# default target redirected to $(TARGET)
all: $(TARGET)

#**************************************
# main target: Use TAB for actions
$(TARGET): $(C_OBJS) $(CC_OBJS)
	$(CC) $^ -o $@ $(LD_FLAGS)

#**************************************
# compile c files using gcc
$(C_OBJS): $(OBJ)/%.o: %.c
	$(MK_INT_DIR)
	$(C) -MMD -MP $(C_FLAGS) $< -o $@
-include $(C_OBJS:.o=.d)

#**************************************
# compile cpp files using g++
$(CC_OBJS): $(OBJ)/%.o: %.cpp
	$(MK_INT_DIR)
	$(CC) -MMD -MP $(CC_FLAGS) $< -o $@
-include $(CC_OBJS:.o=.d)

#**************************************
# avoid conflicts among targets and files
.PHONY: run clean clear force rebuild

#**************************************
# run executable
run: $(TARGET)
	@${TARGET} ${ARGS}

#**************************************
# clean intermediate object files
# ||: mute rm errors for non-existing files
clean:
	$(RM_INT_DIR) ||:

#**************************************
# clean even executable
clear: clean
	$(RM_TARGET) ||:

#**************************************
# force to build
force rebuild: clean $(TARGET)

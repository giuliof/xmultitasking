######################################################################
#                             SOURCES                                #
######################################################################

# Name of the project
PROJ_NAME    := example

## FILES ##
SRCS         := main.c
SRCS         += tasks.c
SRCS         += tasks_asm.S

## Directories ##
## This is where the source files are located,
## which are not in the current directory
SRC_DIR      := ./src

INC_DIR      := $(SRC_DIR)

BUILD_DIR    := build
OUTPUT_DIR   := output

######################################################################
#                          ARCHITECTURE                              #
######################################################################

## GCC toolchain prefix
PREFIX := avr-

## Microcontroller -- AVR
MCU        := atxmega256a3bu
F_CPU      := 2000000
ARCH_FLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU)

######################################################################
#                        TOOLCHAIN FLAGS                             #
######################################################################

## Compiler flags ##
## Debug informations
COMMON_CFLAGS      = -g -gdwarf-2
## Optimisation
# COMMON_CFLAGS    += -Og
COMMON_CFLAGS     += -Os
## All warning messages
COMMON_CFLAGS     += -Wall -Wextra -Wshadow
## Use smallest size for enums
# COMMON_CFLAGS    += -fshort-enums
## Microcontroller informations
COMMON_CFLAGS     += $(ARCH_FLAGS)
## Header files
COMMON_CFLAGS     += $(addprefix -I,$(INC_DIR))
COMMON_CFLAGS     += -DTPRESCALER=1UL -DTICKS_MS=10

## gcc flags ##
CFLAGS = $(COMMON_CFLAGS)
## Puts functions and data into its own section
CFLAGS    += -ffunction-sections -fdata-sections

## g++ flags ##
CXXFLAGS = $(COMMON_CFLAGS)
CXXFLAGS += -std=c++11
CXXFLAGS += -fno-exceptions -fstack-usage -fdump-tree-optimized -ffunction-sections -fdata-sections -fno-threadsafe-statics

## Linker flags ##
LDFLAGS    += $(ARCH_FLAGS)
## Linker script
ifdef LDSCRIPT
LDFLAGS    += -T$(LDSCRIPT)
endif


LDLIBS     :=

######################################################################
#                                                                    #
######################################################################

## Object files
## Automatically declares object file names
OBJS         := $(patsubst %.c,   $(BUILD_DIR)/%.o,  $(filter %.c,$(SRCS)) )
OBJS         += $(patsubst %.cpp, $(BUILD_DIR)/%.o,  $(filter %.cpp,$(SRCS)) )
OBJS         += $(patsubst %.s,   $(BUILD_DIR)/%.o,  $(filter %.s,$(SRCS)) )
OBJS         += $(patsubst %.S,   $(BUILD_DIR)/%.o,  $(filter %.S,$(SRCS)) )

## Dependencies from .h/.hpp files
DEPS         := $(patsubst %.o, %.d, $(OBJS))

## Virtual Paths
## Tell make to look in that folder if it cannot find a source
## from the current directory
vpath %.c   $(SRC_DIR)
vpath %.cpp $(SRC_DIR)
vpath %.S   $(SRC_DIR)
vpath %.s   $(SRC_DIR)
vpath %.h   $(INC_DIR)

######################################################################
#                         SETUP TOOLS                                #
######################################################################
ECHO      := /bin/echo

## GCC/programming Tools
ifdef GCC_PATH
CC      = $(GCC_PATH)/$(PREFIX)gcc
CXX     = $(GCC_PATH)/$(PREFIX)g++
OBJCOPY = $(GCC_PATH)/$(PREFIX)objcopy
OBJDUMP = $(GCC_PATH)/$(PREFIX)objdump
GDB     = $(GCC_PATH)/$(PREFIX)gdb
AS      = $(GCC_PATH)/$(PREFIX)as
SIZE    = $(GCC_PATH)/$(PREFIX)size
else
CC      = $(PREFIX)gcc
CXX     = $(PREFIX)g++
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
GDB     = $(PREFIX)gdb
AS      = $(PREFIX)as
SIZE    = $(PREFIX)size
endif

######################################################################
#                      PROGRAMMING TOOLS                             #
######################################################################
## With avrdude
PRG        = avrdude
PRGFLAGS  += -p atxmega256a3bu -c avrispmkii

######################################################################
#                             TARGETS                                #
######################################################################
all:     $(OUTPUT_DIR)/$(PROJ_NAME).hex

## Avoid generating deps if cleaning
## https://codereview.stackexchange.com/questions/2547/makefile-dependency-generation
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

## asm ##
$(BUILD_DIR)/%.o : %.S
	@echo -e "\033[1;33m[Assembling  ]\033[0m AS $<"
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@

## asm ##
$(BUILD_DIR)/%.o : %.s
	@echo -e "\033[1;33m[Assembling  ]\033[0m AS $<"
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@

## .cxx files ##
$(BUILD_DIR)/%.o:  %.cpp
	@echo -e "\033[1;33m[Compiling   ]\033[0m CX $<"
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) -c $< -o $@

## .c files ##
$(BUILD_DIR)/%.o:  %.c
	@echo -e "\033[1;33m[Compiling   ]\033[0m CC $<"
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@

.DELETE_ON_ERROR:
$(BUILD_DIR)/%.d: %.c
	@mkdir -p `dirname $@`
	@$(ECHO) -n "$@ " > $@
	@$(CC) -MM -MP -MT '$(@D)/$(basename $(<F)).o' $(CFLAGS) $< >> $@

.DELETE_ON_ERROR:
$(BUILD_DIR)/%.d: %.cpp
	@mkdir -p `dirname $@`
	@$(ECHO) -n "$@ " > $@
	@$(CC) -MM -MP -MT '$(@D)/$(basename $(<F)).o' $(CFLAGS) $< >> $@

## Linking ##
$(OUTPUT_DIR)/$(PROJ_NAME).elf: $(OBJS)
	@echo -e "\033[1;34m[Linking     ]\033[0m $@"
	@mkdir -p ${OUTPUT_DIR}
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@echo -e "\033[1;35m[Disasm...   ]\033[0m $^"
	$(OBJDUMP) -S $@ > $(OUTPUT_DIR)/$(PROJ_NAME).lss

## Extracting ##
$(OUTPUT_DIR)/$(PROJ_NAME).hex: $(OUTPUT_DIR)/$(PROJ_NAME).elf
	@echo -e "\033[1;36m[Binary      ]\033[0m $^"
	$(OBJCOPY) -O ihex -R .eeprom $< $@

.PHONY: flash
flash: $(OUTPUT_DIR)/$(PROJ_NAME).hex
	@echo -e "\033[1;36m[Flash      ]\033[0m $^"
	@#$(PRG) $(PRGFLAGS) $<
	$(PRG) $(PRGFLAGS) -U flash:w:$<

.PHONY: size
size: $(OUTPUT_DIR)/$(PROJ_NAME).elf
	$(SIZE) $(OUTPUT_DIR)/$(PROJ_NAME).elf

.PHONY: clean
clean:
	@echo -e "\033[1;33m[Cleaning   ]\033[0m"
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(OUTPUT_DIR)/*

#####################################
#### Tran Viet Thang _ HUST#########

PROJECT = uart-test

#################### MODULE library #####################
MODULES = uart \
		i2c \
		rtc \
		rtos \
		watchdog \
		libwwg \
		rtos/include \
		rtos/portable/ARM_CM3 \
		rtos/portable/MemMang

##########################################################
################### Genlink - config #####################
DEVICE=stm32f103c8t6
############## Debug ###################################
OOCD_INTERFACE ?= stlink-v2
OOCD_TARGET ?= stm32f1x
##########################################################
################### LIBOPENCM3 ###########################
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	OPENCM3_DIR ?= /opt/libopencm3
	#CC_PATH		?= /opt/gcc-arm/bin
endif

##############Check Libopencm3##########################
OPENCM3_EXIST = 0
ifneq ("$(wildcard $(OPENCM3_DIR))","")
	OPENCM3_EXIST = 1
endif

ifneq ("$(wildcard vendor/libopencm3)","")
	OPENCM3_EXIST = 1
	OPENCM3_DIR = ./vendor/libopencm3
endif

##########################################################
ifeq ($(DEVICE),)
$(warning no DEVICE specified for linker script generator)
endif

LDSCRIPT	= generated.$(DEVICE).ld
DEVICES_DATA = $(OPENCM3_DIR)/ld/devices.data

genlink_family		:=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) FAMILY)
genlink_subfamily	:=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) SUBFAMILY)
genlink_cpu			:=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) CPU)
genlink_fpu			:=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) FPU)
genlink_cppflags	:=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) CPPFLAGS)

CPPFLAGS	+= $(genlink_cppflags)

ARCH_FLAGS	:=-mcpu=$(genlink_cpu)
ifeq ($(genlink_cpu),$(filter $(genlink_cpu),cortex-m0 cortex-m0plus cortex-m3 cortex-m4 cortex-m7))
ARCH_FLAGS    +=-mthumb
endif

ifeq ($(genlink_fpu),soft)
ARCH_FLAGS	+= -msoft-float
else ifeq ($(genlink_fpu),hard-fpv4-sp-d16)
ARCH_FLAGS	+= -mfloat-abi=hard -mfpu=fpv4-sp-d16
else ifeq ($(genlink_fpu),hard-fpv5-sp-d16)
ARCH_FLAGS      += -mfloat-abi=hard -mfpu=fpv5-sp-d16
else
$(warning No match for the FPU flags)
endif


ifeq ($(genlink_family),)
$(warning $(DEVICE) not found in $(DEVICES_DATA))
endif

# only append to LDFLAGS if the library file exists to not break builds
# where those are provided by different means
ifneq (,$(wildcard $(OPENCM3_DIR)/lib/libopencm3_$(genlink_family).a))
LIBNAME = opencm3_$(genlink_family)
else
ifneq (,$(wildcard $(OPENCM3_DIR)/lib/libopencm3_$(genlink_subfamily).a))
LIBNAME = opencm3_$(genlink_subfamily)
else
$(warning $(OPENCM3_DIR)/lib/libopencm3_$(genlink_family).a library variant for the selected device does not exist.)
endif
endif

LDLIBS += -l$(LIBNAME)
LIBDEPS += $(OPENCM3_DIR)/lib/lib$(LIBNAME).a

# only append to LDLIBS if the directory exists
ifneq (,$(wildcard $(OPENCM3_DIR)/lib))
LDFLAGS += -L$(OPENCM3_DIR)/lib
else
$(warning $(OPENCM3_DIR)/lib as given be OPENCM3_DIR does not exist.)
endif

# only append include path to CPPFLAGS if the directory exists
ifneq (,$(wildcard $(OPENCM3_DIR)/include))
CPPFLAGS += -I$(OPENCM3_DIR)/include
else
$(warning $(OPENCM3_DIR)/include as given be OPENCM3_DIR does not exist.)
endif
##########################################################
BUILD_DIR ?= bin
OPT ?= -Os
CSTD ?= -std=c99

# Be silent per default, but 'make V=1' will show all compiler calls.
# If you're insane, V=99 will print out all sorts of things.
V?=0
ifeq ($(V),0)
Q	:= @
NULL	:= 2>/dev/null
endif

# Tool paths.
PREFIX	?= arm-none-eabi-
CC	= $(PREFIX)gcc
LD	= $(PREFIX)gcc
OBJCOPY	= $(PREFIX)objcopy
OBJDUMP	= $(PREFIX)objdump
GDB 	= $(PREFIX)gdb
OOCD	?= openocd

OPENCM3_INC = $(OPENCM3_DIR)/include

# Inclusion of library header files
###################Thang comment######################################
INCLUDES += $(patsubst %,-I%, . $(OPENCM3_INC) )

SRC_DIR   := $(MODULES) source
CFILES = $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))

INCLUDES += $(patsubst %,-I%, . $(SRC_DIR))

OBJS = $(CFILES:%.c=$(BUILD_DIR)/%.o)
OBJS += $(AFILES:%.S=$(BUILD_DIR)/%.o)
######################################################################
GENERATED_BINS = $(PROJECT).elf $(PROJECT).bin $(PROJECT).map $(PROJECT).list $(PROJECT).lss

TGT_CPPFLAGS += -MD
TGT_CPPFLAGS += -Wall -Wundef $(INCLUDES)
TGT_CPPFLAGS += $(INCLUDES) $(OPENCM3_DEFS)

TGT_CFLAGS += $(OPT) $(CSTD) -ggdb3
TGT_CFLAGS += $(ARCH_FLAGS)
TGT_CFLAGS += -fno-common
TGT_CFLAGS += -ffunction-sections -fdata-sections
TGT_CFLAGS += -Wextra -Wshadow -Wno-unused-variable -Wimplicit-function-declaration
TGT_CFLAGS += -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes

TGT_CXXFLAGS += $(OPT) $(CXXSTD) -ggdb3
TGT_CXXFLAGS += $(ARCH_FLAGS)
TGT_CXXFLAGS += -fno-common
TGT_CXXFLAGS += -ffunction-sections -fdata-sections
TGT_CXXFLAGS += -Wextra -Wshadow -Wredundant-decls  -Weffc++

TGT_ASFLAGS += $(OPT) $(ARCH_FLAGS) -ggdb3

## Thang Commment 

TGT_LDFLAGS += -u _printf_float

TGT_LDFLAGS += -T$(LDSCRIPT) -L$(OPENCM3_DIR)/lib -nostartfiles
TGT_LDFLAGS += $(ARCH_FLAGS)
TGT_LDFLAGS += -specs=nano.specs
TGT_LDFLAGS += -Wl,--gc-sections
# OPTIONAL
TGT_LDFLAGS += -Wl,-Map=$(PROJECT).map
ifeq ($(V),99)
TGT_LDFLAGS += -Wl,--print-gc-sections
endif

# Linker script generator fills this in for us.
ifeq (,$(DEVICE))
LDLIBS += -l$(OPENCM3_LIB)
endif
# nosys is only in newer gcc-arm-embedded...
#LDLIBS += -specs=nosys.specs
LDLIBS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

# Burn in legacy hell fortran modula pascal yacc idontevenwat
.SUFFIXES:
.SUFFIXES: .c .S .h .o .cxx .elf .bin .list .lss

# Bad make, never *ever* try to get a file out of source control by yourself.
%: %,v
%: RCS/%,v
%: RCS/%
%: s.%
%: SCCS/s.%

all: checkcm3 $(PROJECT).elf $(PROJECT).bin
flash: $(PROJECT).flash

# error if not using linker script generator
ifeq (,$(DEVICE))
$(LDSCRIPT):
ifeq (,$(wildcard $(LDSCRIPT)))
    $(error Unable to find specified linker script: $(LDSCRIPT))
endif
else
# if linker script generator was used, make sure it's cleaned.
GENERATED_BINS += $(LDSCRIPT)
endif

# Need a special rule to have a bin dir
$(BUILD_DIR)/%.o: %.c
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: %.cxx
	@printf "  CXX\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: %.S
	@printf "  AS\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(TGT_ASFLAGS) $(ASFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

$(PROJECT).elf: $(OBJS) $(LDSCRIPT) $(LIBDEPS)
	@printf "  LD\t$@\n"
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

%.bin: %.elf
	@printf "  OBJCOPY\t$@\n"
	$(Q)$(OBJCOPY) -O binary  $< $@

%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

%.list: %.elf
	$(OBJDUMP) -S $< > $@

%.flash: %.elf
	@printf "  FLASH\t$<\n"
ifeq (,$(OOCD_FILE))
	$(Q)(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
		-f target/$(OOCD_TARGET).cfg \
		-c "program $(realpath $(*).elf) verify reset exit" \
		$(NULL)
else
	$(Q)(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f $(OOCD_FILE) \
		-c "program $(realpath $(*).elf) verify reset exit" \
		$(NULL)
endif


$(LDSCRIPT): $(OPENCM3_DIR)/ld/linker.ld.S $(OPENCM3_DIR)/ld/devices.data
	@printf "  GENLNK  $(DEVICE)\n"
	$(Q)$(CPP) $(ARCH_FLAGS) $(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) DEFS) -P -E $< -o $@

debug: 
	$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
	-f target/$(OOCD_TARGET).cfg

gdb: $(PROJECT).elf
	$(GDB) $<
	
clean:
	rm -rf $(BUILD_DIR) $(GENERATED_BINS)

checkcm3:
ifeq ($(OPENCM3_EXIST), 0)
	@git clone https://github.com/libopencm3/libopencm3.git vendor/libopencm3\
	&& cd vendor/libopencm3 && make && cd ../../
endif

.PHONY: all clean flash
-include $(OBJS:.o=.d)

abc : 
	echo "$(CFILES)"
	echo "$(INCLUDES)"
	echo "$(OPENCM3_DIR)"
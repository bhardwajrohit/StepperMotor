#
# Makefile for simple monitor on STM32F3 Discovery Board
#
# openocd -f board/stm32f3discovery.cfg -c "init" -c "halt" -c "flash write_image erase simple_monitor.elf" -c "reset run" -c shutdown


# C source files for the project
PROJ_NAME = simple_monitor
SRCS = main.c

# C to assembly language templates
SRCS += mycode.s mytest.c 

# Hardware Software Interface Lab
SRCS += lab12.c #lab3.c lab4.c lab5.c lab6.c lab7.c lab8.c lab9.c lab10.c lab11.c hello.c

# Simple Monitor sources
SRCS += monitor.c parser.c dump.c syscall.c terminal.c \
	decoder/decoder.c decoder/STM32F30x_decoder.c

# Target board selection, uncomment relevant line
# Rev C boards:
STLINK=stlink-v2-1.cfg
GDB_SCRIPT=gdb_cmds_v2_1
# Rev B boards:
#STLINK=stlink-v2.cfg
#GDB_SCRIPT=gdb_cmds_v2


###################################################
# Location of the linker scripts
LDSCRIPT_INC=ld

# Location of CMSIS files for our device
CMSIS     = Drivers/CMSIS
CMSIS_INC = $(CMSIS)/Include
CMSIS_DEV = $(CMSIS)/Device/ST/STM32F3xx
CMSIS_DEV_INC = $(CMSIS_DEV)/Include
CMSIS_DEV_SRC = $(CMSIS_DEV)/Source/Templates
SRCS += $(CMSIS_DEV_SRC)/system_stm32f3xx.c

# Location of HAL drivers
HAL     = Drivers/STM32F3xx_HAL_Driver
HAL_INC = $(HAL)/Inc
HAL_SRC = $(HAL)/Src
SRCS   += $(HAL_SRC)/stm32f3xx_hal_rcc.c \
          $(HAL_SRC)/stm32f3xx_hal.c \
          $(HAL_SRC)/stm32f3xx_hal_cortex.c \
          $(HAL_SRC)/stm32f3xx_hal_uart.c \
          $(HAL_SRC)/stm32f3xx_hal_gpio.c \
          $(HAL_SRC)/stm32f3xx_hal_pcd.c \
          $(HAL_SRC)/stm32f3xx_hal_pcd_ex.c \
	  $(HAL_SRC)/stm32f3xx_hal_i2c.c \
	  $(HAL_SRC)/stm32f3xx_hal_spi.c \
          $(HAL_SRC)/stm32f3xx_hal_adc.c \
          $(HAL_SRC)/stm32f3xx_hal_adc_ex.c\
          $(HAL_SRC)/stm32f3xx_hal_dac.c  \
          $(HAL_SRC)/stm32f3xx_hal_dac_ex.c \
          $(HAL_SRC)/stm32f3xx_hal_tim.c \
          $(HAL_SRC)/stm32f3xx_hal_tim_ex.c 
 


# USB Sources
USB_CORE     = Drivers/STM32_USB_Device_Library/Core
USB_CORE_INC = $(USB_CORE)/Inc
USB_CORE_SRC = $(USB_CORE)/Src
SRCS   += $(USB_CORE_SRC)/usbd_core.c \
          $(USB_CORE_SRC)/usbd_ctlreq.c \
          $(USB_CORE_SRC)/usbd_ioreq.c

USB_CLASS     = Drivers/STM32_USB_Device_Library/Class/CDC
USB_CLASS_INC = $(USB_CLASS)/Inc
USB_CLASS_SRC = $(USB_CLASS)/Src
SRCS   += $(USB_CLASS_SRC)/usbd_cdc.c \
          usbd_conf.c usbd_desc.c usbd_cdc_interface.c

# Location of BSP Files
BSP     = Drivers/BSP/STM32F3-Discovery
BSP_INC = $(BSP)
BSP_SRC = $(BSP)
SRCS   += $(BSP)/stm32f3_discovery.c \
	  $(BSP)/stm32f3_discovery_accelerometer.c \
	  $(BSP)/stm32f3_discovery_gyroscope.c

# Location of Component Files
CMP     = Drivers/BSP/Components
CMP_INC = $(CMP)
CMP_SRC = $(CMP)
SRCS   += $(CMP)/lsm303dlhc/lsm303dlhc.c \
	  $(CMP)/l3gd20/l3gd20.c


PREFIX	=	arm-none-eabi-
CC=$(PREFIX)gcc
AR=$(PREFIX)ar
AS=$(PREFIX)as
GDB=$(PREFIX)gdb
OBJCOPY=$(PREFIX)objcopy
OBJDUMP=$(PREFIX)objdump
SIZE=$(PREFIX)size

#CFLAGS  = -Wall -g -std=c99 -Os  
CFLAGS  = -Wall -g -std=gnu99
CFLAGS += -Os  
CFLAGS += -Werror
CFLAGS += -mlittle-endian -mcpu=cortex-m4  -march=armv7e-m 
CFLAGS += -mthumb
#CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -I .
CFLAGS += -I $(CMSIS_INC)
CFLAGS += -I $(CMSIS_DEV_INC)
CFLAGS += -DSTM32F303xC
CFLAGS += -I $(HAL_INC)
CFLAGS += -I $(BSP_INC)
CFLAGS += -I $(USB_CORE_INC)
CFLAGS += -I $(USB_CLASS_INC)

LDFLAGS  = -Wall -g -std=c99 -Os  
LDFLAGS += -mlittle-endian -mcpu=cortex-m4  -march=armv7e-m 
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJ_NAME).map
LDFLAGS += -mthumb
#LDFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
LDFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
LDFLAGS += -ffunction-sections -fdata-sections

ASFLAGS =  -g -mlittle-endian -mcpu=cortex-m4  -march=armv7e-m
ASFLAGS += -mthumb
#ASFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
ASFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
###################################################

# add startup file to build	
SRCS += $(CMSIS_DEV_SRC)/gcc/startup_stm32f303xc.s

BUILDDIR = build
OBJS = $(addprefix $(BUILDDIR)/,$(addsuffix .o,$(basename $(SRCS))))
DEPS = $(addprefix deps/,$(addsuffix .d,$(basename $(SRCS))))
DIRS = $(sort $(dir $(OBJS))) $(BUILDDIR)
DIRFILES = $(addsuffix .dir, $(DIRS))

###################################################

.PHONY: all proj program debug clean reallyclean 

all: proj

-include $(DEPS)

proj: 	$(PROJ_NAME).elf

$(DIRFILES): 
	@mkdir -p $(dir $@)
	@touch $@

deps:
	@mkdir -p $@

$(BUILDDIR)/%.o : %.c deps $(DIRFILES)
	$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF deps/$(*F).d

$(BUILDDIR)/%.o : %.s deps $(DIRFILES)
	$(AS) $(ASFLAGS) -c -o $@ $<

$(PROJ_NAME).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ -L$(STD_PERIPH_LIB) -lstm32f3 -L$(LDSCRIPT_INC) -Tsimple_monitor.ld
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	$(OBJDUMP) -St $(PROJ_NAME).elf >$(PROJ_NAME).lst
	$(SIZE) $(PROJ_NAME).elf

program: all
	openocd -f interface/$(STLINK) -f target/stm32f3x.cfg -c "init" -c "reset init" -c "halt" -c "flash write_image erase $(PROJ_NAME).elf" -c "reset run" -c shutdown

debug: program
	$(GDB) --tui -x gdb/$(GDB_SCRIPT) $(PROJ_NAME).elf

ddddebug: program
	ddd --gdb --debugger "$(GDB) -x gdb/$(GDB_SCRIPT)" $(PROJ_NAME).elf

clean:
	find ./ -name '*~' | xargs rm -f
	find ./ -name '*.o' | xargs rm -f
	rm -f deps/*.d
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
	rm -f $(PROJ_NAME).lst
	rm -f openocd.log
	-rmdir deps
	-rm -rf $(BUILDDIR)


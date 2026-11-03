.PHONY: all semihosting load info clean


MODE = nano
ifneq ($(filter semihosting,$(MAKECMDGOALS)),)
MODE = semihosting
endif


INCLUDES  = -I./inc -I./CMSIS/Device/ST/STM32F4xx/Include -I./CMSIS/Include
SRC_PATH  = src
BUILD_ROOT = build
BUILD_DIR = $(BUILD_ROOT)/$(MODE)
SRC_FILES = $(shell find $(SRC_PATH) -iname "*.c")
OBJ_FILES = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

TARGET_NANO  = $(BUILD_ROOT)/final_nano.elf
TARGET_SH 	 = $(BUILD_ROOT)/final_sh.elf

SPECS_NANOLIB 	  = --specs=nano.specs
SPECS_SEMIHOSTING = --specs=rdimon.specs

LD_SCRIPT = stm32_ls.ld
MAP_FILE = $(BUILD_DIR)/final.map
LD_ARGS = -T $(LD_SCRIPT) -Wl,-Map=$(MAP_FILE)


CC      =  arm-none-eabi-gcc
OBJDUMP =  arm-none-eabi-objdump
CFLAGS  = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft


all: $(TARGET_NANO)

$(TARGET_NANO): $(OBJ_FILES) $(BUILD_DIR)/syscalls.o
	$(CC) $(CFLAGS) $(SPECS_NANOLIB) $(LD_ARGS) -Wl,-u,_printf_float $^ -o $@


semihosting: export CFLAGS += -DSEMIHOSTING
semihosting: $(TARGET_SH)

$(TARGET_SH): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(SPECS_SEMIHOSTING) $(LD_ARGS) $^ -o $@


$(BUILD_DIR)/syscalls.o: syscalls.c
	$(CC) -c $(CFLAGS) --std=gnu11 -Wall -O0 $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(INCLUDES) $(CFLAGS) -std=gnu11 -Wall -O0 $^ -o $@


load:
	@cd $(BUILD_ROOT) && openocd -f board/st_nucleo_f4.cfg


info:
	$(OBJDUMP) -h $(BUILD_ROOT)/*.elf


clean:
	rm -rf $(BUILD_ROOT)


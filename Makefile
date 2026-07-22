PROJECT          := hairtos_baremetal
EXAMPLE          ?= 00-baremetal-blink
BUILD_DIR        ?= build/$(EXAMPLE)
TARGET           := $(BUILD_DIR)/$(PROJECT)

TOOLCHAIN        ?= gcc
CROSS_COMPILE    ?= arm-none-eabi-
OPENOCD          ?= openocd
GDB_PORT         ?= 3333

CPU_FLAGS        := -mcpu=cortex-m3 -mthumb
COMMON_FLAGS     := $(CPU_FLAGS) -std=c11 -Og -g3 \
                    -ffreestanding -fno-common -fno-builtin \
                    -fno-unwind-tables -fno-asynchronous-unwind-tables \
                    -ffunction-sections -fdata-sections \
                    -Wall -Wextra -Werror -Wshadow -Wundef \
                    -Wconversion -Wsign-conversion

INCLUDES         := -Iconfig \
                    -Isoc/stm32f1/include \
                    -Iboards/bluepill_f103c8/include \
                    -Idrivers/gpio/include \
                    -Idrivers/uart/include \
                    -Idrivers/timer/include

LINKER_SCRIPT    := boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld
OPENOCD_CFG      := tools/openocd/bluepill_stlink.cfg

C_SOURCES        := soc/stm32f1/system_stm32f1.c \
                    soc/stm32f1/stm32f1_clock.c \
                    soc/stm32f1/stm32f1_irq.c \
                    boards/bluepill_f103c8/board.c \
                    boards/bluepill_f103c8/board_clock.c \
                    drivers/gpio/src/hr_gpio_stm32f1.c \
                    drivers/uart/src/hr_uart_stm32f1.c \
                    drivers/timer/src/hr_hw_timer_stm32f1.c \
                    examples/$(EXAMPLE)/main.c

ASM_SOURCES      := soc/stm32f1/startup_stm32f103.S

C_OBJECTS        := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
ASM_OBJECTS      := $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.S=.o))
OBJECTS          := $(C_OBJECTS) $(ASM_OBJECTS)
DEPS             := $(C_OBJECTS:.o=.d)

ifeq ($(TOOLCHAIN),clang)
CC               := clang
OBJCOPY          := llvm-objcopy
SIZE             := size
OBJDUMP          := llvm-objdump
GDB              := gdb-multiarch
TOOLCHAIN_FLAGS  := --target=arm-none-eabi
LD_DRIVER_FLAGS  := --target=arm-none-eabi -fuse-ld=lld
LDLIBS           :=
else
CC               := $(CROSS_COMPILE)gcc
OBJCOPY          := $(CROSS_COMPILE)objcopy
SIZE             := $(CROSS_COMPILE)size
OBJDUMP          := $(CROSS_COMPILE)objdump
GDB              := $(CROSS_COMPILE)gdb
TOOLCHAIN_FLAGS  :=
LD_DRIVER_FLAGS  :=
LDLIBS           := -lgcc
endif

CFLAGS           := $(TOOLCHAIN_FLAGS) $(COMMON_FLAGS) $(INCLUDES) -MMD -MP
ASFLAGS          := $(TOOLCHAIN_FLAGS) $(CPU_FLAGS) -g3 -x assembler-with-cpp $(INCLUDES)
LDFLAGS          := $(LD_DRIVER_FLAGS) $(CPU_FLAGS) -nostdlib \
                    -Wl,--gc-sections -Wl,-Map=$(TARGET).map \
                    -Wl,--cref -T$(LINKER_SCRIPT)

.PHONY: all elf bin hex size flash erase debug-server gdb disasm \
        phase0-check phase1-check roadmap-check tree clean help

all: elf bin hex size

elf: $(TARGET).elf

bin: $(TARGET).bin

hex: $(TARGET).hex

$(TARGET).elf: $(OBJECTS) $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

size: $(TARGET).elf
	$(SIZE) $<

flash: $(TARGET).elf
	$(OPENOCD) -f $(OPENOCD_CFG) \
		-c "program $(abspath $<) verify reset exit"

erase:
	$(OPENOCD) -f $(OPENOCD_CFG) \
		-c "init" -c "reset halt" -c "stm32f1x mass_erase 0" -c "shutdown"

debug-server:
	$(OPENOCD) -f $(OPENOCD_CFG)

gdb: $(TARGET).elf
	$(GDB) -q $< -x tools/gdb/hairtos.gdb

disasm: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).lst
	@echo "Generated $(TARGET).lst"

phase0-check:
	python3 tools/scripts/phase0_check.py

phase1-check:
	python3 tools/scripts/phase1_check.py

roadmap-check:
	python3 tools/scripts/roadmap_check.py

tree:
	@find . -path './.git' -prune -o -path './build' -prune -o -print | sort

clean:
	rm -rf build out

help:
	@echo "HairRTOS Phase 1 commands"
	@echo "  make                         Build the bare-metal example with GCC"
	@echo "  make TOOLCHAIN=clang         Build with Clang/LLD"
	@echo "  make flash                   Build, verify, flash, and reset"
	@echo "  make erase                   Mass erase STM32F1 flash"
	@echo "  make debug-server            Start OpenOCD"
	@echo "  make gdb                     Connect GDB to localhost:3333"
	@echo "  make disasm                  Generate source/interleaved listing"
	@echo "  make phase1-check            Validate and build Phase 1 files"
	@echo "  make roadmap-check           Validate future roadmap specifications"
	@echo "  make clean                   Remove build output"

-include $(DEPS)

PROJECT          := hairtos_baremetal
EXAMPLE          ?= 01-baremetal-foundation
BUILD_DIR        ?= build/$(EXAMPLE)
TARGET           := $(BUILD_DIR)/$(PROJECT)

TARGET_EXAMPLES  := 01-baremetal-foundation 03-static-task-stack 04-start-first-task 05-cooperative-context-switch 06-priority-scheduler 07-task-delay-timeout 08-preemption-round-robin 09-queue-blocking-ipc 10-01-semaphore-from-isr 10-02-mutex-priority-inheritance 11-task-suspend-resume 12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo
HOST_EXAMPLES    := 02-kernel-data-structures-host
FIRMWARE_GOALS   := all elf bin hex size flash gdb disasm
REQUESTED_FW     := $(filter $(FIRMWARE_GOALS),$(MAKECMDGOALS))
ifeq ($(strip $(MAKECMDGOALS)),)
REQUESTED_FW     := all
endif
ifneq ($(strip $(REQUESTED_FW)),)
ifeq ($(filter $(EXAMPLE),$(TARGET_EXAMPLES)),)
$(error EXAMPLE=$(EXAMPLE) is not an implemented STM32 target example. Use 'make phase2-example' for the Phase 2 host demo)
endif
endif

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
                    -Idrivers/timer/include \
                    -Ikernel/include -Ikernel/internal \
                    -Iarch/arm/cortex-m3/include \
                    -Ihairevent/include -Ihairevent/internal

LINKER_SCRIPT    := boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld
OPENOCD_CFG      := tools/openocd/bluepill_stlink.cfg

PLATFORM_C_SOURCES := soc/stm32f1/system_stm32f1.c \
                      soc/stm32f1/stm32f1_clock.c \
                      soc/stm32f1/stm32f1_irq.c \
                      boards/bluepill_f103c8/board.c \
                      boards/bluepill_f103c8/board_clock.c \
                      drivers/gpio/src/hr_gpio_stm32f1.c \
                      drivers/uart/src/hr_uart_stm32f1.c \
                      drivers/timer/src/hr_hw_timer_stm32f1.c

BAREMETAL_TICK_SOURCE := drivers/timer/src/hr_systick_baremetal_irq.c

PHASE3_C_SOURCES := kernel/src/hr_list.c \
                    kernel/src/hr_scheduler.c \
                    kernel/src/hr_wait.c \
                    kernel/src/hr_timeout.c \
                    kernel/src/hr_task.c \
                    arch/arm/cortex-m3/hr_port.c \
                    arch/arm/cortex-m3/hr_port_stack.c

C_SOURCES        := $(PLATFORM_C_SOURCES) examples/$(EXAMPLE)/main.c
ASM_SOURCES      := soc/stm32f1/startup_stm32f103.S

ifneq ($(filter $(EXAMPLE),03-static-task-stack 04-start-first-task 05-cooperative-context-switch 06-priority-scheduler 07-task-delay-timeout 08-preemption-round-robin 09-queue-blocking-ipc 10-01-semaphore-from-isr 10-02-mutex-priority-inheritance 11-task-suspend-resume 12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo),)
C_SOURCES        += $(PHASE3_C_SOURCES)
endif

ifneq ($(filter $(EXAMPLE),04-start-first-task 05-cooperative-context-switch 06-priority-scheduler 07-task-delay-timeout 08-preemption-round-robin 09-queue-blocking-ipc 10-01-semaphore-from-isr 10-02-mutex-priority-inheritance 11-task-suspend-resume 12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo),)
C_SOURCES        += kernel/src/hr_kernel.c
ASM_SOURCES      += arch/arm/cortex-m3/hr_portasm.S
endif

ifneq ($(filter $(EXAMPLE),01-baremetal-foundation 03-static-task-stack 04-start-first-task 05-cooperative-context-switch 06-priority-scheduler),)
C_SOURCES        += $(BAREMETAL_TICK_SOURCE)
endif

ifneq ($(filter $(EXAMPLE),07-task-delay-timeout 08-preemption-round-robin 09-queue-blocking-ipc 10-01-semaphore-from-isr 10-02-mutex-priority-inheritance 11-task-suspend-resume 12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo),)
C_SOURCES        += kernel/src/hr_time.c
endif

ifeq ($(EXAMPLE),09-queue-blocking-ipc)
C_SOURCES        += kernel/src/hr_queue.c
endif

ifeq ($(EXAMPLE),10-01-semaphore-from-isr)
C_SOURCES        += kernel/src/hr_semaphore.c
endif

ifeq ($(EXAMPLE),10-02-mutex-priority-inheritance)
C_SOURCES        += kernel/src/hr_mutex.c
endif

ifeq ($(EXAMPLE),12-software-timer)
C_SOURCES        += kernel/src/hr_semaphore.c kernel/src/hr_timer.c
endif

ifneq ($(filter $(EXAMPLE),13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo),)
C_SOURCES        += kernel/src/hr_context.c \
                    kernel/src/hr_queue.c \
                    kernel/src/hr_semaphore.c \
                    kernel/src/hr_timer.c \
                    hairevent/src/he_event.c \
                    hairevent/src/he_state_machine.c \
                    hairevent/src/he_active.c \
                    hairevent/src/he_time_event.c \
                    hairevent/src/he_pubsub.c
endif

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

EXAMPLE_DEFINES  :=
ifeq ($(EXAMPLE),07-task-delay-timeout)
EXAMPLE_DEFINES  += -DHR_CFG_PREEMPTION=0 -DHR_CFG_TIME_SLICING=0
endif
ifneq ($(filter $(EXAMPLE),12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo),)
EXAMPLE_DEFINES  += -DHR_CFG_ENABLE_SOFTWARE_TIMER=1 -DHR_CFG_TIMER_TASK_PRIORITY=1
else
EXAMPLE_DEFINES  += -DHR_CFG_ENABLE_SOFTWARE_TIMER=0
endif

CFLAGS           := $(TOOLCHAIN_FLAGS) $(COMMON_FLAGS) $(EXAMPLE_DEFINES) $(INCLUDES) -MMD -MP
ASFLAGS          := $(TOOLCHAIN_FLAGS) $(CPU_FLAGS) -g3 -x assembler-with-cpp $(INCLUDES)

HOST_CC          ?= cc
HOST_BUILD_DIR   ?= build/host
HOST_TEST        := $(HOST_BUILD_DIR)/completed_phase_tests
PHASE2_EXAMPLE   := $(HOST_BUILD_DIR)/02-kernel-data-structures-host
HOST_FLAGS       := -std=c11 -O0 -g3 -Wall -Wextra -Werror -Wshadow -Wundef \
                    -Wconversion -Wsign-conversion -pedantic \
                    -fsanitize=address,undefined -fno-omit-frame-pointer
HOST_INCLUDES    := -Iconfig -Ikernel/include -Ikernel/internal -Itests/host \
                    -Iarch/arm/cortex-m3/include -Ihairevent/include -Ihairevent/internal
HOST_SOURCES     := kernel/src/hr_list.c \
                    kernel/src/hr_scheduler.c \
                    kernel/src/hr_wait.c \
                    kernel/src/hr_timeout.c \
                    kernel/src/hr_task.c \
                    kernel/src/hr_kernel.c \
                    kernel/src/hr_time.c \
                    kernel/src/hr_queue.c \
                    kernel/src/hr_semaphore.c \
                    kernel/src/hr_mutex.c \
                    kernel/src/hr_timer.c \
                    kernel/src/hr_context.c \
                    hairevent/src/he_event.c \
                    hairevent/src/he_state_machine.c \
                    hairevent/src/he_active.c \
                    hairevent/src/he_time_event.c \
                    hairevent/src/he_pubsub.c \
                    arch/arm/cortex-m3/hr_port_stack.c \
                    tests/mocks/mock_port.c \
                    tests/host/test_main.c \
                    tests/host/test_list.c \
                    tests/host/test_ready_queue.c \
                    tests/host/test_wait_list.c \
                    tests/host/test_timeout.c \
                    tests/host/test_port_stack.c \
                    tests/host/test_task.c \
                    tests/host/test_kernel_start.c \
                    tests/host/test_scheduler_policy.c \
                    tests/host/test_queue.c \
                    tests/host/test_semaphore.c \
                    tests/host/test_mutex.c \
                    tests/host/test_timer.c \
                    tests/host/test_hairevent.c

LDFLAGS          := $(LD_DRIVER_FLAGS) $(CPU_FLAGS) -nostdlib \
                    -Wl,--gc-sections -Wl,-Map=$(TARGET).map \
                    -Wl,--cref -T$(LINKER_SCRIPT)

.PHONY: all elf bin hex size flash erase debug-server gdb disasm \
        phase0-check phase1-check roadmap-check example-layout-check \
        phase2-check phase3-check phase4-check phase5-check phase6-check phase7-check phase8-check phase9-check phase10-check phase11-check phase12-check phase13-check host-tests phase2-example \
        tree clean help

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

$(HOST_TEST): $(HOST_SOURCES)
	@mkdir -p $(HOST_BUILD_DIR)
	$(HOST_CC) $(HOST_FLAGS) $(HOST_INCLUDES) $(HOST_SOURCES) -o $@

host-tests: $(HOST_TEST)
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=halt_on_error=1 $(HOST_TEST)

$(PHASE2_EXAMPLE): kernel/src/hr_list.c kernel/src/hr_scheduler.c kernel/src/hr_wait.c \
		examples/02-kernel-data-structures-host/main.c
	@mkdir -p $(HOST_BUILD_DIR)
	$(HOST_CC) $(HOST_FLAGS) $(HOST_INCLUDES) \
		kernel/src/hr_list.c kernel/src/hr_scheduler.c kernel/src/hr_wait.c \
		examples/02-kernel-data-structures-host/main.c -o $@

phase2-example: $(PHASE2_EXAMPLE)
	$(PHASE2_EXAMPLE)

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

example-layout-check:
	python3 tools/scripts/example_layout_check.py

phase2-check:
	python3 tools/scripts/phase2_check.py

phase3-check:
	python3 tools/scripts/phase3_check.py

phase4-check:
	python3 tools/scripts/phase4_check.py

phase5-check:
	python3 tools/scripts/phase5_check.py

phase6-check:
	python3 tools/scripts/phase6_check.py

phase7-check:
	python3 tools/scripts/phase7_check.py

phase8-check:
	python3 tools/scripts/phase8_check.py

phase9-check:
	python3 tools/scripts/phase9_check.py

phase10-check:
	python3 tools/scripts/phase10_check.py

phase11-check:
	python3 tools/scripts/phase11_check.py

phase12-check:
	python3 tools/scripts/phase12_check.py

phase13-check:
	python3 tools/scripts/phase13_check.py

tree:
	@find . -path './.git' -prune -o -path './build' -prune -o -print | sort

clean:
	rm -rf build out

help:
	@echo "HairRTOS Phase 13 commands"
	@echo "  make EXAMPLE=01-baremetal-foundation       Build Phase 1 target"
	@echo "  make phase2-example                         Run Phase 2 host demo"
	@echo "  make EXAMPLE=03-static-task-stack           Build Phase 3 target"
	@echo "  make EXAMPLE=04-start-first-task            Build Phase 4 target"
	@echo "  make EXAMPLE=04-start-first-task flash      Flash Phase 4 target"
	@echo "  make EXAMPLE=05-cooperative-context-switch  Build Phase 5 target"
	@echo "  make EXAMPLE=05-cooperative-context-switch flash  Flash Phase 5 target"
	@echo "  make EXAMPLE=06-priority-scheduler        Build Phase 6 target"
	@echo "  make EXAMPLE=06-priority-scheduler flash  Flash Phase 6 target"
	@echo "  make EXAMPLE=07-task-delay-timeout      Build Phase 7 target"
	@echo "  make EXAMPLE=07-task-delay-timeout flash  Flash Phase 7 target"
	@echo "  make EXAMPLE=08-preemption-round-robin    Build Phase 8 target"
	@echo "  make EXAMPLE=08-preemption-round-robin flash  Flash Phase 8 target"
	@echo "  make EXAMPLE=09-queue-blocking-ipc       Build Phase 9 target"
	@echo "  make EXAMPLE=09-queue-blocking-ipc flash Flash Phase 9 target"
	@echo "  make EXAMPLE=10-01-semaphore-from-isr      Build Phase 10 ISR semaphore target"
	@echo "  make EXAMPLE=10-01-semaphore-from-isr flash  Flash Phase 10 ISR semaphore target"
	@echo "  make EXAMPLE=10-02-mutex-priority-inheritance  Build Phase 10 mutex target"
	@echo "  make EXAMPLE=10-02-mutex-priority-inheritance flash  Flash Phase 10 mutex target"
	@echo "  make EXAMPLE=11-task-suspend-resume          Build Phase 11 target"
	@echo "  make EXAMPLE=11-task-suspend-resume flash    Flash Phase 11 target"
	@echo "  make EXAMPLE=12-software-timer               Build Phase 12 target"
	@echo "  make EXAMPLE=12-software-timer flash         Flash Phase 12 target"
	@echo "  make EXAMPLE=13-01-event-post                Build Phase 13 event-post target"
	@echo "  make EXAMPLE=13-02-active-object             Build Phase 13 Active Object target"
	@echo "  make EXAMPLE=13-03-flat-state-machine        Build Phase 13 flat-SM target"
	@echo "  make EXAMPLE=13-04-time-event                Build Phase 13 time-event target"
	@echo "  make EXAMPLE=13-05-publish-subscribe         Build Phase 13 pub/sub target"
	@echo "  make EXAMPLE=13-06-event-driven-demo         Build Phase 13 integration target"
	@echo "  make host-tests                              Run Phase 2–13 host tests"
	@echo "  make phase13-check                           Validate completed phases"
	@echo "  make roadmap-check                           Validate roadmap status"
	@echo "  make example-layout-check                    Validate example matrix"
	@echo "  make clean                                   Remove build output"

-include $(DEPS)

.DEFAULT_GOAL := build

PROJECT          := hairtos
EXAMPLE          ?= hairtos
ENVIRONMENT      ?= auto
BUILD_ROOT       ?= build

TARGET_EXAMPLES  := 01-baremetal-foundation \
                    03-static-task-stack \
                    04-start-first-task \
                    05-cooperative-context-switch \
                    06-priority-scheduler \
                    07-task-delay-timeout \
                    08-preemption-round-robin \
                    09-queue-blocking-ipc \
                    10-01-semaphore-from-isr \
                    10-02-mutex-priority-inheritance \
                    11-task-suspend-resume \
                    12-software-timer \
                    13-01-event-post \
                    13-02-active-object \
                    13-03-flat-state-machine \
                    13-04-time-event \
                    13-05-publish-subscribe \
                    13-06-event-driven-demo \
                    14-memory-allocator-lab \
                    15-kernel-benchmark \
                    hairtos

HOST_EXAMPLES    := 02-kernel-data-structures-host \
                    14-memory-allocator-lab \
                    hairtos

HOST_ONLY_EXAMPLES := 02-kernel-data-structures-host
DUAL_EXAMPLES      := 14-memory-allocator-lab hairtos
ALL_EXAMPLES       := $(sort $(TARGET_EXAMPLES) $(HOST_EXAMPLES))

ifeq ($(ENVIRONMENT),auto)
ifneq ($(filter $(EXAMPLE),$(HOST_ONLY_EXAMPLES)),)
RESOLVED_PLATFORM := host
else
RESOLVED_PLATFORM := target
endif
else ifeq ($(ENVIRONMENT),host)
RESOLVED_PLATFORM := host
else ifeq ($(ENVIRONMENT),target)
RESOLVED_PLATFORM := target
else
$(error ENVIRONMENT must be auto, host, or target)
endif

TARGET_BUILD_DIR ?= $(BUILD_ROOT)/target/$(EXAMPLE)
HOST_BUILD_DIR   ?= $(BUILD_ROOT)/host/$(EXAMPLE)
BUILD_DIR         := $(TARGET_BUILD_DIR)
TARGET            := $(TARGET_BUILD_DIR)/$(PROJECT)

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
                    -Ihairevent/include -Ihairevent/internal \
                    -Ilabs/memory-allocator/include \
                    -Ibenchmarks/kernel/include

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

ifneq ($(filter $(EXAMPLE),03-static-task-stack 04-start-first-task 05-cooperative-context-switch 06-priority-scheduler 07-task-delay-timeout 08-preemption-round-robin 09-queue-blocking-ipc 10-01-semaphore-from-isr 10-02-mutex-priority-inheritance 11-task-suspend-resume 12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo 15-kernel-benchmark hairtos),)
C_SOURCES        += $(PHASE3_C_SOURCES)
endif

ifneq ($(filter $(EXAMPLE),04-start-first-task 05-cooperative-context-switch 06-priority-scheduler 07-task-delay-timeout 08-preemption-round-robin 09-queue-blocking-ipc 10-01-semaphore-from-isr 10-02-mutex-priority-inheritance 11-task-suspend-resume 12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo 15-kernel-benchmark hairtos),)
C_SOURCES        += kernel/src/hr_kernel.c
ASM_SOURCES      += arch/arm/cortex-m3/hr_portasm.S
endif

ifneq ($(filter $(EXAMPLE),01-baremetal-foundation 03-static-task-stack 04-start-first-task 05-cooperative-context-switch 06-priority-scheduler),)
C_SOURCES        += $(BAREMETAL_TICK_SOURCE)
endif

ifneq ($(filter $(EXAMPLE),07-task-delay-timeout 08-preemption-round-robin 09-queue-blocking-ipc 10-01-semaphore-from-isr 10-02-mutex-priority-inheritance 11-task-suspend-resume 12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo 15-kernel-benchmark hairtos),)
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

ifeq ($(EXAMPLE),15-kernel-benchmark)
C_SOURCES        += kernel/src/hr_context.c \
                    kernel/src/hr_queue.c \
                    kernel/src/hr_semaphore.c \
                    kernel/src/hr_mutex.c \
                    kernel/src/hr_timer.c \
                    hairevent/src/he_event.c \
                    hairevent/src/he_state_machine.c \
                    hairevent/src/he_active.c \
                    benchmarks/kernel/src/hr_benchmark_stats.c \
                    benchmarks/kernel/src/hr_benchmark_dwt.c \
                    benchmarks/kernel/src/hr_benchmark_gpio.c
endif

ifeq ($(EXAMPLE),hairtos)
C_SOURCES        += kernel/src/hr_context.c \
                    kernel/src/hr_queue.c \
                    kernel/src/hr_semaphore.c \
                    kernel/src/hr_mutex.c \
                    kernel/src/hr_timer.c \
                    kernel/src/hr_diagnostics.c \
                    arch/arm/cortex-m3/hr_fault.c
ASM_SOURCES      += arch/arm/cortex-m3/hr_faultasm.S
endif

ifeq ($(EXAMPLE),14-memory-allocator-lab)
C_SOURCES        += labs/memory-allocator/src/hr_heap_lab.c \
                    labs/memory-allocator/src/hr_pool_lab.c
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
ifeq ($(EXAMPLE),hairtos)
EXAMPLE_DEFINES  += -DHR_CFG_PREEMPTION=1 -DHR_CFG_TIME_SLICING=1 \
                    -DHR_CFG_ENABLE_SOFTWARE_TIMER=1 \
                    -DHR_CFG_TIMER_TASK_PRIORITY=1 \
                    -DHR_CFG_ENABLE_DIAGNOSTICS=1 \
                    -DHR_CFG_ENABLE_RUNTIME_STATS=1
else ifeq ($(EXAMPLE),15-kernel-benchmark)
EXAMPLE_DEFINES  += -DHR_CFG_PREEMPTION=1 -DHR_CFG_TIME_SLICING=0 \
                    -DHR_CFG_ENABLE_SOFTWARE_TIMER=1 \
                    -DHR_CFG_TIMER_TASK_PRIORITY=1
else ifneq ($(filter $(EXAMPLE),12-software-timer 13-01-event-post 13-02-active-object 13-03-flat-state-machine 13-04-time-event 13-05-publish-subscribe 13-06-event-driven-demo),)
EXAMPLE_DEFINES  += -DHR_CFG_ENABLE_SOFTWARE_TIMER=1 -DHR_CFG_TIMER_TASK_PRIORITY=1
else
EXAMPLE_DEFINES  += -DHR_CFG_ENABLE_SOFTWARE_TIMER=0
endif

CFLAGS           := $(TOOLCHAIN_FLAGS) $(COMMON_FLAGS) $(EXAMPLE_DEFINES) $(EXTRA_DEFINES) $(INCLUDES) -MMD -MP
ASFLAGS          := $(TOOLCHAIN_FLAGS) $(CPU_FLAGS) -g3 -x assembler-with-cpp $(INCLUDES)


HOST_CC          ?= cc
HOST_TEST_BUILD_DIR := $(BUILD_ROOT)/host/tests
HOST_TEST        := $(HOST_TEST_BUILD_DIR)/hairtos_tests
HOST_BINARY      := $(HOST_BUILD_DIR)/$(EXAMPLE)
HOST_EXAMPLE_SOURCES :=

ifeq ($(EXAMPLE),02-kernel-data-structures-host)
HOST_EXAMPLE_SOURCES := kernel/src/hr_list.c \
                        kernel/src/hr_scheduler.c \
                        kernel/src/hr_wait.c \
                        examples/02-kernel-data-structures-host/main.c
endif

ifeq ($(EXAMPLE),14-memory-allocator-lab)
HOST_EXAMPLE_SOURCES := labs/memory-allocator/src/hr_heap_lab.c \
                        labs/memory-allocator/src/hr_pool_lab.c \
                        labs/memory-allocator/demo.c
endif

ifeq ($(EXAMPLE),hairtos)
HOST_EXAMPLE_SOURCES := kernel/src/hr_list.c \
                        kernel/src/hr_scheduler.c \
                        tests/stress/scheduler_stress_core.c \
                        tests/stress/scheduler_stress_main.c
endif

HOST_FLAGS       := -std=c11 -O0 -g3 -Wall -Wextra -Werror -Wshadow -Wundef \
                    -Wconversion -Wsign-conversion -pedantic \
                    -fsanitize=address,undefined -fno-omit-frame-pointer \
                    -DHR_CFG_ENABLE_DIAGNOSTICS=1 -DHR_CFG_ENABLE_RUNTIME_STATS=1
HOST_INCLUDES    := -Iconfig -Ikernel/include -Ikernel/internal -Itests/host \
                    -Iarch/arm/cortex-m3/include -Ihairevent/include -Ihairevent/internal \
                    -Ilabs/memory-allocator/include -Ibenchmarks/kernel/include -Itests/stress
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
                    labs/memory-allocator/src/hr_heap_lab.c \
                    labs/memory-allocator/src/hr_pool_lab.c \
                    benchmarks/kernel/src/hr_benchmark_stats.c \
                    kernel/src/hr_diagnostics.c \
                    tests/stress/scheduler_stress_core.c \
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
                    tests/host/test_hairevent.c \
                    tests/host/test_benchmark.c \
                    tests/host/test_diagnostics.c \
                    tests/stress/test_scheduler_stress.c \
                    labs/memory-allocator/tests/test_heap_lab.c

LDFLAGS          := $(LD_DRIVER_FLAGS) $(CPU_FLAGS) -nostdlib \
                    -Wl,--gc-sections -Wl,-Map=$(TARGET).map \
                    -Wl,--cref -T$(LINKER_SCRIPT)

.PHONY: all build run check clean clean-all help list-examples tree \
        target-build target-run host-build host-run host-tests \
        elf bin hex size flash erase debug-server gdb disasm \
        validate-example validate-target-example validate-host-example

all: build

ifeq ($(RESOLVED_PLATFORM),host)
build: host-build
run: host-run
check: host-tests host-build
else
build: target-build
run: target-run
check: host-tests target-build
endif

target-build: elf bin hex size
target-run: flash
host-build: $(HOST_BINARY)
host-run: $(HOST_BINARY)
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=halt_on_error=1 $(HOST_BINARY)

validate-example:
	@case " $(ALL_EXAMPLES) " in \
		*" $(EXAMPLE) "*) ;; \
		*) echo "Error: unknown EXAMPLE=$(EXAMPLE)"; \
		   echo "Run 'make list-examples' to see valid names."; exit 2 ;; \
	esac

validate-target-example: validate-example
	@case " $(TARGET_EXAMPLES) " in \
		*" $(EXAMPLE) "*) ;; \
		*) echo "Error: EXAMPLE=$(EXAMPLE) is not available for ENVIRONMENT=target."; \
		   echo "Use ENVIRONMENT=host for a host example."; exit 2 ;; \
	esac

validate-host-example: validate-example
	@case " $(HOST_EXAMPLES) " in \
		*" $(EXAMPLE) "*) ;; \
		*) echo "Error: EXAMPLE=$(EXAMPLE) is not available for ENVIRONMENT=host."; \
		   echo "Use ENVIRONMENT=target for an STM32 example."; exit 2 ;; \
	esac

elf: $(TARGET).elf
bin: $(TARGET).bin
hex: $(TARGET).hex

$(TARGET).elf: $(OBJECTS) $(LINKER_SCRIPT) | validate-target-example
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(TARGET_BUILD_DIR)/%.o: %.c | validate-target-example
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_BUILD_DIR)/%.o: %.S | validate-target-example
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

size: $(TARGET).elf
	$(SIZE) $<

$(HOST_BINARY): $(HOST_EXAMPLE_SOURCES) | validate-host-example
	@mkdir -p $(HOST_BUILD_DIR)
	$(HOST_CC) $(HOST_FLAGS) $(HOST_INCLUDES) $(HOST_EXAMPLE_SOURCES) -o $@

$(HOST_TEST): $(HOST_SOURCES)
	@mkdir -p $(HOST_TEST_BUILD_DIR)
	$(HOST_CC) $(HOST_FLAGS) $(HOST_INCLUDES) $(HOST_SOURCES) -o $@

host-tests: $(HOST_TEST)
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	UBSAN_OPTIONS=halt_on_error=1 $(HOST_TEST)

flash: $(TARGET).elf
	$(OPENOCD) -f $(OPENOCD_CFG) \
		-c "program $(abspath $<) verify reset exit"

erase: validate-target-example
	$(OPENOCD) -f $(OPENOCD_CFG) \
		-c "init" -c "reset halt" -c "stm32f1x mass_erase 0" -c "shutdown"

debug-server: validate-target-example
	$(OPENOCD) -f $(OPENOCD_CFG)

gdb: $(TARGET).elf
	$(GDB) -q $< -x tools/gdb/hairtos.gdb

disasm: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).lst
	@echo "Generated $(TARGET).lst"

tree:
	@find . -path './.git' -prune -o -path './build' -prune -o -print | sort

clean:
ifeq ($(origin EXAMPLE),command line)
	@echo "Cleaning $(RESOLVED_PLATFORM) example: $(EXAMPLE)"
	@rm -rf $(if $(filter host,$(RESOLVED_PLATFORM)),$(HOST_BUILD_DIR),$(TARGET_BUILD_DIR))
else
	@echo "Cleaning all generated files"
	@rm -rf $(BUILD_ROOT) out
endif

clean-all:
	rm -rf $(BUILD_ROOT) out

list-examples:
	@echo "Host only:"
	@printf '  %s\n' $(HOST_ONLY_EXAMPLES)
	@echo "Target only:"
	@for item in $(filter-out $(DUAL_EXAMPLES),$(TARGET_EXAMPLES)); do echo "  $$item"; done
	@echo "Host + target:"
	@printf '  %s\n' $(DUAL_EXAMPLES)

help:
	@echo "hairtos Makefile"
	@echo "  make EXAMPLE=<name> build [ENVIRONMENT=host|target]"
	@echo "  make EXAMPLE=<name> run   [ENVIRONMENT=host|target]"
	@echo "  make EXAMPLE=<name> check [ENVIRONMENT=host|target]"
	@echo "  make EXAMPLE=<name> clean [ENVIRONMENT=host|target]"
	@echo "  make host-tests | list-examples | clean-all"
	@echo "Auto mode: host-only examples use host; all others use target."

-include $(DEPS)

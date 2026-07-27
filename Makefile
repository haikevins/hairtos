
.DEFAULT_GOAL := build

PROJECT       := hairtos
DEFAULT_TARGET := $(strip $(shell cmake -P cmake/print_default_target.cmake 2>/dev/null))
ifeq ($(origin TARGET),command line)
HAIRTOS_TARGET := $(TARGET)
else
HAIRTOS_TARGET ?= $(DEFAULT_TARGET)
endif
EXAMPLE       ?= $(strip $(shell cmake -P cmake/print_default_example.cmake 2>/dev/null))
ENVIRONMENT   ?= auto
BUILD_ROOT    ?= build
TOOLCHAIN     ?= gcc
OPENOCD       ?= openocd
EXTRA_DEFINES ?=

RESOLVED_ENVIRONMENT := $(strip $(shell cmake \
    -DHAIRTOS_EXAMPLE=$(EXAMPLE) \
    -DHAIRTOS_ENVIRONMENT=$(ENVIRONMENT) \
    -P cmake/resolve_environment.cmake 2>/dev/null))

ifeq ($(RESOLVED_ENVIRONMENT),)
$(error Invalid EXAMPLE or ENVIRONMENT. Run 'make list-examples')
endif

TARGET_DESCRIPTION := $(strip $(shell cmake \
    -DHAIRTOS_TARGET=$(HAIRTOS_TARGET) \
    -DHAIRTOS_PROPERTY=DESCRIPTION \
    -P cmake/print_target_property.cmake 2>/dev/null))
OPENOCD_CFG := $(strip $(shell cmake \
    -DHAIRTOS_TARGET=$(HAIRTOS_TARGET) \
    -DHAIRTOS_PROPERTY=OPENOCD_CONFIG \
    -P cmake/print_target_property.cmake 2>/dev/null))
OPENOCD_ERASE_COMMAND := $(strip $(shell cmake \
    -DHAIRTOS_TARGET=$(HAIRTOS_TARGET) \
    -DHAIRTOS_PROPERTY=OPENOCD_ERASE_COMMAND \
    -P cmake/print_target_property.cmake 2>/dev/null))

ifeq ($(TARGET_DESCRIPTION),)
$(error Invalid TARGET '$(HAIRTOS_TARGET)'. Run 'make list-targets')
endif

ifeq ($(RESOLVED_ENVIRONMENT),target)
BUILD_DIR := $(BUILD_ROOT)/target/$(HAIRTOS_TARGET)/$(EXAMPLE)
INTELLISENSE_DIR := $(BUILD_ROOT)/intellisense/target/$(HAIRTOS_TARGET)/$(EXAMPLE)
else
BUILD_DIR := $(BUILD_ROOT)/host/$(EXAMPLE)
INTELLISENSE_DIR := $(BUILD_ROOT)/intellisense/host/$(EXAMPLE)
endif

TARGET_ELF := $(BUILD_DIR)/$(PROJECT).elf
HOST_BINARY := $(BUILD_DIR)/$(EXAMPLE)
HOST_TEST_DIR := $(BUILD_ROOT)/host/tests

ifeq ($(TOOLCHAIN),clang)
TOOLCHAIN_FILE := cmake/toolchains/arm-none-eabi-clang.cmake
GDB ?= gdb-multiarch
else ifeq ($(TOOLCHAIN),gcc)
TOOLCHAIN_FILE := cmake/toolchains/arm-none-eabi-gcc.cmake
GDB ?= arm-none-eabi-gdb
else
$(error TOOLCHAIN must be gcc or clang)
endif

TARGET_CONFIGURE = cmake -S . -B $(BUILD_DIR) -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
    -DHAIRTOS_ENVIRONMENT=target \
    -DHAIRTOS_TARGET=$(HAIRTOS_TARGET) \
    -DHAIRTOS_EXAMPLE=$(EXAMPLE) \
    -DHAIRTOS_EXTRA_C_FLAGS="$(EXTRA_DEFINES)"

HOST_CONFIGURE = cmake -S . -B $(BUILD_DIR) -G Ninja \
    -DHAIRTOS_ENVIRONMENT=host \
    -DHAIRTOS_TARGET=$(HAIRTOS_TARGET) \
    -DHAIRTOS_EXAMPLE=$(EXAMPLE) \
    -DHAIRTOS_BUILD_TESTS=OFF \
    -DHAIRTOS_EXTRA_C_FLAGS="$(EXTRA_DEFINES)"

HOST_TEST_CONFIGURE = cmake -S . -B $(HOST_TEST_DIR) -G Ninja \
    -DHAIRTOS_ENVIRONMENT=host \
    -DHAIRTOS_TARGET=$(HAIRTOS_TARGET) \
    -DHAIRTOS_EXAMPLE=16-diagnostics-stress-stabilization \
    -DHAIRTOS_BUILD_TESTS=ON

ifeq ($(RESOLVED_ENVIRONMENT),host)
INTELLISENSE_CONFIGURE = cmake -S . -B $(INTELLISENSE_DIR) -G Ninja \
    -DHAIRTOS_ENVIRONMENT=host \
    -DHAIRTOS_TARGET=$(HAIRTOS_TARGET) \
    -DHAIRTOS_EXAMPLE=$(EXAMPLE) \
    -DHAIRTOS_BUILD_TESTS=OFF \
    -DHAIRTOS_EXTRA_C_FLAGS="$(EXTRA_DEFINES)"
else
INTELLISENSE_CONFIGURE = cmake -S . -B $(INTELLISENSE_DIR) -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
    -DHAIRTOS_ENVIRONMENT=target \
    -DHAIRTOS_TARGET=$(HAIRTOS_TARGET) \
    -DHAIRTOS_EXAMPLE=$(EXAMPLE) \
    -DHAIRTOS_EXTRA_C_FLAGS="$(EXTRA_DEFINES)"
endif

.PHONY: all build run check clean clean-all help list-examples list-targets tree \
        target-build target-run host-build host-run host-tests \
        size flash erase debug-server gdb disasm intellisense

all: build

ifeq ($(RESOLVED_ENVIRONMENT),host)
build: host-build
run: host-run
else
build: target-build
run: target-run
endif

check: host-tests build

target-build:
	@$(TARGET_CONFIGURE)
	@ln -sfn $(abspath $(BUILD_DIR))/compile_commands.json compile_commands.json
	@cmake --build $(BUILD_DIR) --target hairtos
	@cmake --build $(BUILD_DIR) --target size

host-build:
	@$(HOST_CONFIGURE)
	@ln -sfn $(abspath $(BUILD_DIR))/compile_commands.json compile_commands.json
	@cmake --build $(BUILD_DIR) --target hairtos_host

host-run: host-build
	@ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
	 UBSAN_OPTIONS=halt_on_error=1 $(HOST_BINARY)

target-run: target-build
	$(OPENOCD) -f $(OPENOCD_CFG) \
		-c "program $(abspath $(TARGET_ELF)) verify reset exit"

host-tests:
	@$(HOST_TEST_CONFIGURE)
	@cmake --build $(HOST_TEST_DIR) --target hairtos_tests
	@ctest --test-dir $(HOST_TEST_DIR) --output-on-failure

intellisense:
	@$(INTELLISENSE_CONFIGURE)
	@ln -sfn $(abspath $(INTELLISENSE_DIR))/compile_commands.json compile_commands.json
	@echo "IntelliSense database: compile_commands.json"
	@echo "Target: $(HAIRTOS_TARGET) - $(TARGET_DESCRIPTION)"

size:
	@$(TARGET_CONFIGURE)
	@cmake --build $(BUILD_DIR) --target size

flash: target-run

disasm:
	@$(TARGET_CONFIGURE)
	@cmake --build $(BUILD_DIR) --target disasm
	@echo "Generated $(BUILD_DIR)/hairtos.lst"

erase:
	$(OPENOCD) -f $(OPENOCD_CFG) \
		-c "init" -c "reset halt" -c "$(OPENOCD_ERASE_COMMAND)" -c "shutdown"

debug-server:
	$(OPENOCD) -f $(OPENOCD_CFG)

gdb: target-build
	$(GDB) -q $(TARGET_ELF) -x tools/gdb/hairtos.gdb

clean:
ifeq ($(origin EXAMPLE),command line)
	@echo "Cleaning $(RESOLVED_ENVIRONMENT) example: $(EXAMPLE)"
	@rm -rf $(BUILD_DIR)
else
	@echo "Cleaning all generated files"
	@rm -rf $(BUILD_ROOT) out compile_commands.json
endif

clean-all:
	@rm -rf $(BUILD_ROOT) out compile_commands.json

list-examples:
	@cmake -P cmake/print_examples.cmake

list-targets:
	@cmake -P cmake/print_targets.cmake

tree:
	@find . -path './.git' -prune -o -path './build' -prune -o -print | sort

help:
	@echo "hairtos"
	@echo "  make TARGET=<target> EXAMPLE=<name> build [ENVIRONMENT=host|target]"
	@echo "  make TARGET=<target> EXAMPLE=<name> run   [ENVIRONMENT=host|target]"
	@echo "  make TARGET=<target> EXAMPLE=<name> check [ENVIRONMENT=host|target]"
	@echo "  make TARGET=<target> EXAMPLE=<name> clean [ENVIRONMENT=host|target]"
	@echo "  make TARGET=<target> EXAMPLE=<name> intellisense"
	@echo "  make host-tests | list-targets | list-examples | clean-all"

#!/usr/bin/env python3
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "soc/stm32f1/startup_stm32f103.S",
    "soc/stm32f1/system_stm32f1.c",
    "soc/stm32f1/stm32f1_clock.c",
    "soc/stm32f1/include/stm32f1.h",
    "boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld",
    "boards/bluepill_f103c8/board.c",
    "drivers/gpio/src/hr_gpio_stm32f1.c",
    "drivers/uart/src/hr_uart_stm32f1.c",
    "drivers/timer/src/hr_hw_timer_stm32f1.c",
    "drivers/timer/src/hr_systick_baremetal_irq.c",
    "examples/01-baremetal-foundation/main.c",
    "tools/openocd/bluepill_stlink.cfg",
    "docs/roadmap.md",
    "docs/task-suspend-resume.md",
    "docs/memory-allocator-lab.md",
    "docs/benchmark-plan.md",
]

missing = [item for item in required if not (ROOT / item).is_file()]
empty = [item for item in required if (ROOT / item).is_file() and (ROOT / item).stat().st_size == 0]

if missing or empty:
    for item in missing:
        print(f"missing: {item}")
    for item in empty:
        print(f"empty: {item}")
    sys.exit(1)

print("HairRTOS Phase 1 structure check: PASS")

if shutil.which("clang") and shutil.which("ld.lld") and shutil.which("llvm-objcopy"):
    subprocess.run(["make", "clean"], cwd=ROOT, check=True)
    subprocess.run(["make", "TOOLCHAIN=clang", "all"], cwd=ROOT, check=True)
    print("HairRTOS Phase 1 Clang build: PASS")
else:
    print("Clang validation skipped: required LLVM tools were not found")

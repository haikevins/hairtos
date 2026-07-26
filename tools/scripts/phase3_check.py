#!/usr/bin/env python3

from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "kernel/include/hairtos/hr_task.h",
    "kernel/internal/hr_task_internal.h",
    "kernel/src/hr_task.c",
    "arch/arm/cortex-m3/include/hr_port.h",
    "arch/arm/cortex-m3/include/hr_portmacro.h",
    "arch/arm/cortex-m3/hr_port.c",
    "arch/arm/cortex-m3/hr_port_stack.c",
    "tests/mocks/mock_port.c",
    "tests/host/test_task.c",
    "tests/host/test_port_stack.c",
    "examples/03-static-task-stack/main.c",
    "docs/phase3-tcb-initial-stack.md",
]

missing = [item for item in required if not (ROOT / item).is_file()]
empty = [item for item in required if (ROOT / item).is_file() and
         (ROOT / item).stat().st_size == 0]
if missing or empty:
    for item in missing:
        print(f"missing: {item}")
    for item in empty:
        print(f"empty: {item}")
    sys.exit(1)

subprocess.run(["python3", "tools/scripts/example_layout_check.py"], cwd=ROOT, check=True)

host_cc = "clang" if shutil.which("clang") else "cc"
subprocess.run(["make", "clean"], cwd=ROOT, check=True)
subprocess.run(["make", f"HOST_CC={host_cc}", "host-tests"], cwd=ROOT, check=True)

if shutil.which("clang") and shutil.which("ld.lld") and shutil.which("llvm-objcopy"):
    subprocess.run(["make", "TOOLCHAIN=clang", "EXAMPLE=01-baremetal-foundation", "all"],
                   cwd=ROOT, check=True)
    subprocess.run(["make", "clean"], cwd=ROOT, check=True)
    subprocess.run(["make", "TOOLCHAIN=clang", "EXAMPLE=03-static-task-stack", "all"],
                   cwd=ROOT, check=True)
    print("HairRTOS Phase 1 target regression build: PASS")
    print("HairRTOS Phase 3 target build: PASS")
else:
    print("Target builds skipped: LLVM tools were not found")

print("HairRTOS Phase 3 structure check: PASS")
print("HairRTOS Phase 3 host tests: PASS")

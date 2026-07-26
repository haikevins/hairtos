#!/usr/bin/env python3

from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "kernel/internal/hr_list_internal.h",
    "kernel/internal/hr_scheduler_internal.h",
    "kernel/internal/hr_wait_internal.h",
    "kernel/internal/hr_timeout_internal.h",
    "kernel/src/hr_list.c",
    "kernel/src/hr_scheduler.c",
    "kernel/src/hr_wait.c",
    "kernel/src/hr_timeout.c",
    "tests/host/test_support.h",
    "tests/host/test_main.c",
    "tests/host/test_list.c",
    "tests/host/test_ready_queue.c",
    "tests/host/test_wait_list.c",
    "tests/host/test_timeout.c",
    "docs/phase2-kernel-data-structures.md",
    "examples/02-kernel-data-structures-host/main.c",
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

for source in [ROOT / "kernel/src/hr_list.c",
               ROOT / "kernel/src/hr_scheduler.c",
               ROOT / "kernel/src/hr_wait.c",
               ROOT / "kernel/src/hr_timeout.c"]:
    content = source.read_text(encoding="utf-8")
    if "stm32" in content.lower():
        print(f"MCU dependency found in Phase 2 source: {source}")
        sys.exit(1)

host_cc = "clang" if shutil.which("clang") else "cc"
subprocess.run(["make", "clean"], cwd=ROOT, check=True)
subprocess.run(["make", f"HOST_CC={host_cc}", "host-tests"], cwd=ROOT, check=True)
subprocess.run(["make", f"HOST_CC={host_cc}", "phase2-example"], cwd=ROOT, check=True)

if shutil.which("clang") and shutil.which("ld.lld") and shutil.which("llvm-objcopy"):
    subprocess.run(["make", "TOOLCHAIN=clang", "all"], cwd=ROOT, check=True)
    print("HairRTOS Phase 1 target regression build: PASS")
else:
    print("Target regression build skipped: LLVM tools were not found")

print("HairRTOS Phase 2 structure check: PASS")
print("HairRTOS Phase 2 host tests: PASS")

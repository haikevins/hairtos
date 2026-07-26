#!/usr/bin/env python3
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
clang = shutil.which("clang")
if clang is None:
    print("Cortex-M0 compile proof skipped: clang not found")
    sys.exit(0)

build = ROOT / "build/cortex-m0-proof"
build.mkdir(parents=True, exist_ok=True)
common = [
    clang, "--target=arm-none-eabi", "-mcpu=cortex-m0", "-mthumb",
    "-std=c11", "-ffreestanding", "-fno-builtin", "-Wall", "-Wextra",
    "-Werror", "-Wshadow", "-Wundef", "-Wconversion", "-Wsign-conversion",
    "-Iconfig", "-Ikernel/include", "-Ikernel/internal",
]
sources = [
    "kernel/src/hr_list.c",
    "kernel/src/hr_scheduler.c",
    "kernel/src/hr_wait.c",
    "kernel/src/hr_timeout.c",
    "tests/portability/cortex_m0_compile_probe.c",
]
for source in sources:
    output = build / (Path(source).stem + ".o")
    subprocess.run(common + ["-c", source, "-o", str(output)],
                   cwd=ROOT, check=True)

print("HairRTOS Cortex-M0 architecture-independent compile proof: PASS")
print("Scope: intrusive lists, scheduler data structures, wait/timeout core, public types")
print("Not claimed: SVC/PendSV context-switch assembly or a board runtime port")

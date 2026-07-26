#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "kernel/src/hr_kernel.c",
    "kernel/src/hr_task.c",
    "kernel/internal/hr_kernel_internal.h",
    "arch/arm/cortex-m3/hr_port.c",
    "arch/arm/cortex-m3/hr_portasm.S",
    "tests/host/test_kernel_start.c",
    "examples/05-cooperative-context-switch/main.c",
    "examples/05-cooperative-context-switch/README.md",
    "docs/phase5-cooperative-context-switch.md",
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

subprocess.run(["python3", "tools/scripts/example_layout_check.py"],
               cwd=ROOT, check=True)

host_cc = "clang" if shutil.which("clang") else "cc"
subprocess.run(["make", "clean"], cwd=ROOT, check=True)
subprocess.run(["make", f"HOST_CC={host_cc}", "host-tests"], cwd=ROOT, check=True)

built_target = False
examples = [
    "01-baremetal-foundation",
    "03-static-task-stack",
    "04-start-first-task",
    "05-cooperative-context-switch",
]
if shutil.which("clang") and shutil.which("ld.lld") and shutil.which("llvm-objcopy"):
    for example in examples:
        subprocess.run(["make", "clean"], cwd=ROOT, check=True)
        subprocess.run(["make", "TOOLCHAIN=clang", f"EXAMPLE={example}", "all"],
                       cwd=ROOT, check=True)
    built_target = True
elif shutil.which("arm-none-eabi-gcc") and shutil.which("arm-none-eabi-objcopy"):
    for example in examples:
        subprocess.run(["make", "clean"], cwd=ROOT, check=True)
        subprocess.run(["make", f"EXAMPLE={example}", "all"], cwd=ROOT, check=True)
    built_target = True
else:
    print("Target builds skipped: no complete ARM cross toolchain was found")

if built_target:
    elf = ROOT / "build/05-cooperative-context-switch/hairtos_baremetal.elf"
    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = shutil.which("llvm-objdump") or shutil.which("arm-none-eabi-objdump")

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        for required_symbol in ["PendSV_Handler", "hr_port_request_context_switch",
                                "hr_kernel_select_next_from_pendsv",
                                "g_hr_current_task_control_block"]:
            if required_symbol not in symbols:
                print(f"missing target symbol: {required_symbol}")
                sys.exit(1)

        pendsv_lines = [line for line in symbols.splitlines() if "PendSV_Handler" in line]
        if not pendsv_lines or not any(re.search(r"\bT\b", line) for line in pendsv_lines):
            print("PendSV_Handler is not a strong text symbol")
            sys.exit(1)

    if objdump_tool:
        disassembly = subprocess.check_output([objdump_tool, "-d", str(elf)], text=True)
        lowered = disassembly.lower()
        checks = [
            "<pendsv_handler>",
            "psp",
            "stmdb",
            "ldm.w",
            "hr_kernel_select_next_from_pendsv",
        ]
        for item in checks:
            if item not in lowered:
                print(f"PendSV disassembly missing: {item}")
                sys.exit(1)

    size_tool = shutil.which("size") or shutil.which("arm-none-eabi-size")
    if size_tool:
        print(subprocess.check_output([size_tool, str(elf)], text=True).strip())

    print("HairRTOS Phase 1/3/4 target regression builds: PASS")
    print("HairRTOS Phase 5 PendSV target build: PASS")
    print("HairRTOS Phase 5 PendSV symbols/disassembly: PASS")

print("HairRTOS Phase 5 structure check: PASS")
print("HairRTOS Phase 5 host tests: PASS")
print("Physical STM32F103 cooperative-switch runtime validation: required on target hardware")

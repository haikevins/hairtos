#!/usr/bin/env python3

from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "kernel/include/hairtos/hr_kernel.h",
    "kernel/internal/hr_kernel_internal.h",
    "kernel/src/hr_kernel.c",
    "kernel/src/hr_task.c",
    "arch/arm/cortex-m3/hr_port.c",
    "arch/arm/cortex-m3/hr_portasm.S",
    "tests/host/test_kernel_start.c",
    "examples/04-start-first-task/main.c",
    "examples/04-start-first-task/README.md",
    "docs/phase4-start-first-task-svc.md",
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
if shutil.which("clang") and shutil.which("ld.lld") and shutil.which("llvm-objcopy"):
    for example in ["01-baremetal-foundation", "03-static-task-stack", "04-start-first-task"]:
        subprocess.run(["make", "clean"], cwd=ROOT, check=True)
        subprocess.run(["make", "TOOLCHAIN=clang", f"EXAMPLE={example}", "all"],
                       cwd=ROOT, check=True)
    built_target = True
elif shutil.which("arm-none-eabi-gcc") and shutil.which("arm-none-eabi-objcopy"):
    for example in ["01-baremetal-foundation", "03-static-task-stack", "04-start-first-task"]:
        subprocess.run(["make", "clean"], cwd=ROOT, check=True)
        subprocess.run(["make", f"EXAMPLE={example}", "all"], cwd=ROOT, check=True)
    built_target = True
else:
    print("Target builds skipped: no complete ARM cross toolchain was found")

if built_target:
    elf = ROOT / "build/04-start-first-task/hairtos_baremetal.elf"
    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = shutil.which("llvm-objdump") or shutil.which("arm-none-eabi-objdump")

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        for required_symbol in ["SVC_Handler", "hr_port_start_first_task",
                                "g_hr_current_task_control_block"]:
            if required_symbol not in symbols:
                print(f"missing target symbol: {required_symbol}")
                sys.exit(1)

    if objdump_tool:
        disassembly = subprocess.check_output([objdump_tool, "-d", str(elf)], text=True)
        required_mnemonics = ["svc", "msr", "psp", "msp"]
        for mnemonic in required_mnemonics:
            if mnemonic not in disassembly.lower():
                print(f"SVC startup disassembly missing: {mnemonic}")
                sys.exit(1)

    print("HairRTOS Phase 1/3 target regression builds: PASS")
    print("HairRTOS Phase 4 SVC target build: PASS")
    print("HairRTOS Phase 4 SVC symbols/disassembly: PASS")

print("HairRTOS Phase 4 structure check: PASS")
print("HairRTOS Phase 4 host tests: PASS")
print("Physical STM32F103 SVC runtime validation: required on target hardware")

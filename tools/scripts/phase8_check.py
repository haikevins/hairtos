#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "kernel/src/hr_kernel.c",
    "kernel/src/hr_scheduler.c",
    "kernel/internal/hr_scheduler_internal.h",
    "kernel/src/hr_task.c",
    "arch/arm/cortex-m3/hr_portasm.S",
    "tests/host/test_kernel_start.c",
    "tests/host/test_scheduler_policy.c",
    "examples/08-preemption-round-robin/main.c",
    "examples/08-preemption-round-robin/README.md",
    "docs/phase8-preemption-round-robin.md",
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

kernel_text = (ROOT / "kernel/src/hr_kernel.c").read_text(encoding="utf-8")
scheduler_text = (ROOT / "kernel/src/hr_scheduler.c").read_text(encoding="utf-8")
assembly_text = (ROOT / "arch/arm/cortex-m3/hr_portasm.S").read_text(encoding="utf-8").lower()

required_kernel_tokens = [
    "HR_SWITCH_REASON_PREEMPT",
    "HR_SWITCH_REASON_TIME_SLICE",
    "hr_scheduler_should_preempt",
    "hr_scheduler_has_equal_priority_peer",
    "time_slice_remaining--",
    "hr_port_yield_from_isr",
]
for token in required_kernel_tokens:
    if token not in kernel_text:
        print(f"Phase 8 kernel token missing: {token}")
        sys.exit(1)

for token in ["hr_scheduler_should_preempt", "hr_scheduler_has_equal_priority_peer"]:
    if token not in scheduler_text:
        print(f"Phase 8 scheduler token missing: {token}")
        sys.exit(1)

for token in ["pendsv_handler:", "stmdb", "ldmia", "cpsid i", "cpsie i"]:
    if token not in assembly_text:
        print(f"Phase 8 PendSV token missing: {token}")
        sys.exit(1)

subprocess.run(["python3", "tools/scripts/phase0_check.py"], cwd=ROOT, check=True)
subprocess.run(["python3", "tools/scripts/example_layout_check.py"], cwd=ROOT, check=True)
subprocess.run(["python3", "tools/scripts/roadmap_check.py"], cwd=ROOT, check=True)

host_compilers = []
if shutil.which("clang"):
    host_compilers.append("clang")
if shutil.which("gcc"):
    host_compilers.append("gcc")
if not host_compilers:
    host_compilers.append("cc")

for host_cc in host_compilers:
    subprocess.run(["make", "clean"], cwd=ROOT, check=True)
    subprocess.run(["make", f"HOST_CC={host_cc}", "host-tests"], cwd=ROOT, check=True)

examples = [
    "01-baremetal-foundation",
    "03-static-task-stack",
    "04-start-first-task",
    "05-cooperative-context-switch",
    "06-priority-scheduler",
    "07-task-delay-timeout",
    "08-preemption-round-robin",
]

built_target = False
if shutil.which("clang") and shutil.which("ld.lld") and shutil.which("llvm-objcopy"):
    build_prefix = ["make", "TOOLCHAIN=clang"]
    built_target = True
elif shutil.which("arm-none-eabi-gcc") and shutil.which("arm-none-eabi-objcopy"):
    build_prefix = ["make"]
    built_target = True
else:
    build_prefix = []
    print("Target builds skipped: no complete ARM cross toolchain was found")

if built_target:
    for example in examples:
        subprocess.run(["make", "clean"], cwd=ROOT, check=True)
        subprocess.run(build_prefix + [f"EXAMPLE={example}", "all"],
                       cwd=ROOT, check=True)

    elf = ROOT / "build/08-preemption-round-robin/hairtos_baremetal.elf"
    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = shutil.which("llvm-objdump") or shutil.which("arm-none-eabi-objdump")

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        required_symbols = [
            "SysTick_Handler",
            "PendSV_Handler",
            "hr_kernel_tick_from_isr",
            "hr_kernel_select_next_from_pendsv",
            "hr_scheduler_should_preempt",
            "hr_scheduler_has_equal_priority_peer",
        ]
        for symbol in required_symbols:
            if symbol not in symbols:
                print(f"missing target symbol: {symbol}")
                sys.exit(1)

        for handler in ["SysTick_Handler", "PendSV_Handler"]:
            lines = [line for line in symbols.splitlines() if handler in line]
            if not lines or not any(re.search(r"\bT\b", line) for line in lines):
                print(f"{handler} is not a strong text symbol")
                sys.exit(1)

    if objdump_tool:
        disassembly = subprocess.check_output([objdump_tool, "-d", str(elf)], text=True)
        lowered = disassembly.lower()
        for item in [
            "<systick_handler>",
            "<pendsv_handler>",
            "hr_kernel_tick_from_isr",
            "hr_kernel_select_next_from_pendsv",
            "hr_scheduler_should_preempt",
            "hr_scheduler_has_equal_priority_peer",
            "cpsid i",
            "cpsie i",
        ]:
            if item not in lowered:
                print(f"Phase 8 disassembly missing: {item}")
                sys.exit(1)

    size_tool = shutil.which("size") or shutil.which("arm-none-eabi-size")
    if size_tool:
        print(subprocess.check_output([size_tool, str(elf)], text=True).strip())

# Host-only example must be rejected before a target build starts.
rejected = subprocess.run(
    ["make", "EXAMPLE=02-kernel-data-structures-host", "flash"],
    cwd=ROOT,
    text=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
)
if rejected.returncode == 0 or "not an implemented STM32 target example" not in rejected.stdout:
    print("Host-only flash rejection check failed")
    print(rejected.stdout)
    sys.exit(1)

print("HairRTOS Phase 8 structure check: PASS")
print("HairRTOS Phase 8 host tests: PASS")
if built_target:
    print("HairRTOS Phase 1/3/4/5/6/7 target regression builds: PASS")
    print("HairRTOS Phase 8 preemption/round-robin target build: PASS")
    print("HairRTOS Phase 8 symbols/disassembly: PASS")
print("Physical STM32F103 preemption runtime validation: required on target hardware")

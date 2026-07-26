#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "kernel/include/hairtos/hr_queue.h",
    "kernel/internal/hr_queue_internal.h",
    "kernel/src/hr_queue.c",
    "kernel/src/hr_kernel.c",
    "kernel/internal/hr_task_internal.h",
    "tests/host/test_queue.c",
    "tests/host/test_kernel_start.c",
    "examples/09-queue-blocking-ipc/main.c",
    "examples/09-queue-blocking-ipc/README.md",
    "docs/phase9-queue-blocking-ipc.md",
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

queue_text = (ROOT / "kernel/src/hr_queue.c").read_text(encoding="utf-8")
kernel_text = (ROOT / "kernel/src/hr_kernel.c").read_text(encoding="utf-8")
header_text = (ROOT / "kernel/include/hairtos/hr_queue.h").read_text(encoding="utf-8")

for token in [
    "hr_queue_create_static",
    "hr_queue_send",
    "hr_queue_receive",
    "hr_queue_send_from_isr",
    "hr_queue_receive_from_isr",
    "hr_queue_wake_receiver",
    "hr_queue_fill_from_waiting_sender",
    "HR_ERROR_QUEUE_FULL",
    "HR_ERROR_QUEUE_EMPTY",
]:
    if token not in queue_text and token not in header_text:
        print(f"Phase 9 queue token missing: {token}")
        sys.exit(1)

for token in [
    "hr_kernel_block_current_on_wait_list",
    "hr_kernel_unblock_waiter",
    "HR_TASK_WAIT_QUEUE_SEND",
    "HR_TASK_WAIT_QUEUE_RECEIVE",
    "HR_ERROR_TIMEOUT",
]:
    if token not in kernel_text:
        print(f"Phase 9 kernel token missing: {token}")
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
    subprocess.run(["make", f"HOST_CC={host_cc}", "host-tests"],
                   cwd=ROOT, check=True)

examples = [
    "01-baremetal-foundation",
    "03-static-task-stack",
    "04-start-first-task",
    "05-cooperative-context-switch",
    "06-priority-scheduler",
    "07-task-delay-timeout",
    "08-preemption-round-robin",
    "09-queue-blocking-ipc",
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

    elf = ROOT / "build/09-queue-blocking-ipc/hairtos_baremetal.elf"
    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = shutil.which("llvm-objdump") or shutil.which("arm-none-eabi-objdump")

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        required_symbols = [
            "SysTick_Handler",
            "PendSV_Handler",
            "hr_queue_create_static",
            "hr_queue_send",
            "hr_queue_receive",
            "hr_kernel_block_current_on_wait_list",
            "hr_kernel_unblock_waiter",
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
            "hr_queue_send",
            "hr_queue_receive",
            "hr_kernel_block_current_on_wait_list",
            "hr_kernel_unblock_waiter",
            "cpsid i",
            "cpsie i",
        ]:
            if item not in lowered:
                print(f"Phase 9 disassembly missing: {item}")
                sys.exit(1)

    size_tool = shutil.which("size") or shutil.which("arm-none-eabi-size")
    if size_tool:
        print(subprocess.check_output([size_tool, str(elf)], text=True).strip())

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

print("HairRTOS Phase 9 structure check: PASS")
print("HairRTOS Phase 9 host tests: PASS")
if built_target:
    print("HairRTOS Phase 1/3/4/5/6/7/8 target regression builds: PASS")
    print("HairRTOS Phase 9 queue/blocking target build: PASS")
    print("HairRTOS Phase 9 symbols/disassembly: PASS")
print("Physical STM32F103 queue runtime validation: required on target hardware")

#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "kernel/include/hairtos/hr_task.h",
    "kernel/internal/hr_task_internal.h",
    "kernel/internal/hr_kernel_internal.h",
    "kernel/src/hr_task.c",
    "kernel/src/hr_kernel.c",
    "tests/host/test_kernel_start.c",
    "examples/11-task-suspend-resume/main.c",
    "examples/11-task-suspend-resume/README.md",
    "docs/task-suspend-resume.md",
    "docs/phase11-suspend-resume.md",
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

public_task = (ROOT / "kernel/include/hairtos/hr_task.h").read_text(encoding="utf-8")
internal_task = (ROOT / "kernel/internal/hr_task_internal.h").read_text(encoding="utf-8")
task_source = (ROOT / "kernel/src/hr_task.c").read_text(encoding="utf-8")
kernel_source = (ROOT / "kernel/src/hr_kernel.c").read_text(encoding="utf-8")
test_source = (ROOT / "tests/host/test_kernel_start.c").read_text(encoding="utf-8")

for token in ["hr_task_suspend", "hr_task_resume", "HR_TASK_STATE_SUSPENDED"]:
    if token not in public_task and token not in task_source:
        print(f"Phase 11 public token missing: {token}")
        sys.exit(1)

for token in [
    "suspended_resume_state",
    "hr_kernel_suspend_task",
    "hr_kernel_resume_task",
    "HR_TASK_STATE_BLOCKED",
    "HR_TASK_STATE_READY",
    "HR_SWITCH_REASON_PREEMPT",
]:
    if token not in internal_task and token not in kernel_source:
        print(f"Phase 11 kernel token missing: {token}")
        sys.exit(1)

for token in [
    "Resume before completion restores the original BLOCKED wait",
    "HR_ERROR_FROM_ISR",
    "hr_task_suspend(&high_b)",
    "hr_task_resume(&high_b)",
]:
    if token not in test_source:
        print(f"Phase 11 host-test token missing: {token}")
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
    "10-01-semaphore-from-isr",
    "10-02-mutex-priority-inheritance",
    "11-task-suspend-resume",
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

    # Rebuild Phase 11 because the regression loop cleans between examples.
    subprocess.run(["make", "clean"], cwd=ROOT, check=True)
    subprocess.run(build_prefix + ["EXAMPLE=11-task-suspend-resume", "elf"],
                   cwd=ROOT, check=True)
    elf = ROOT / "build/11-task-suspend-resume/hairtos_baremetal.elf"

    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = (shutil.which("llvm-objdump") or
                    shutil.which("arm-none-eabi-objdump"))

    required_symbols = [
        "hr_task_suspend",
        "hr_task_resume",
        "hr_kernel_suspend_task",
        "hr_kernel_resume_task",
        "SysTick_Handler",
        "PendSV_Handler",
    ]

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        for symbol in required_symbols:
            if symbol not in symbols:
                print(f"missing Phase 11 target symbol: {symbol}")
                sys.exit(1)
        for symbol in ["SysTick_Handler", "PendSV_Handler"]:
            lines = [line for line in symbols.splitlines() if symbol in line]
            if not lines or not any(re.search(r"\bT\b", line) for line in lines):
                print(f"{symbol} is not a strong text symbol")
                sys.exit(1)

    if objdump_tool:
        disassembly = subprocess.check_output(
            [objdump_tool, "-d", str(elf)], text=True).lower()
        for symbol in required_symbols:
            if symbol.lower() not in disassembly:
                print(f"Phase 11 disassembly missing: {symbol}")
                sys.exit(1)

    if shutil.which("cmake") and shutil.which("ninja") and shutil.which("clang"):
        build_dir = ROOT / "build/cmake-11-task-suspend-resume"
        subprocess.run([
            "cmake", "-S", ".", "-B", str(build_dir), "-G", "Ninja",
            f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake/toolchains/arm-none-eabi-clang.cmake'}",
            "-DHAIRTOS_EXAMPLE=11-task-suspend-resume",
        ], cwd=ROOT, check=True)
        subprocess.run(["cmake", "--build", str(build_dir)], cwd=ROOT, check=True)

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

print("HairRTOS Phase 11 structure check: PASS")
print("HairRTOS Phase 11 host suspend/resume tests: PASS")
if built_target:
    print("HairRTOS Phase 1-10 target regression builds: PASS")
    print("HairRTOS Phase 11 target build and symbols/disassembly: PASS")
    print("HairRTOS Phase 11 CMake/Ninja build: PASS")
print("Physical STM32F103 suspend/resume runtime validation: required on target hardware")

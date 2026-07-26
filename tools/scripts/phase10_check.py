#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "kernel/include/hairtos/hr_semaphore.h",
    "kernel/internal/hr_semaphore_internal.h",
    "kernel/src/hr_semaphore.c",
    "kernel/include/hairtos/hr_mutex.h",
    "kernel/internal/hr_mutex_internal.h",
    "kernel/src/hr_mutex.c",
    "kernel/internal/hr_task_internal.h",
    "kernel/src/hr_kernel.c",
    "tests/host/test_semaphore.c",
    "tests/host/test_mutex.c",
    "tests/host/test_kernel_start.c",
    "examples/10-01-semaphore-from-isr/main.c",
    "examples/10-01-semaphore-from-isr/README.md",
    "examples/10-02-mutex-priority-inheritance/main.c",
    "examples/10-02-mutex-priority-inheritance/README.md",
    "docs/phase10-semaphore-mutex.md",
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

semaphore_text = (ROOT / "kernel/src/hr_semaphore.c").read_text(encoding="utf-8")
mutex_text = (ROOT / "kernel/src/hr_mutex.c").read_text(encoding="utf-8")
kernel_text = (ROOT / "kernel/src/hr_kernel.c").read_text(encoding="utf-8")
task_text = (ROOT / "kernel/internal/hr_task_internal.h").read_text(encoding="utf-8")

for token in [
    "hr_semaphore_create_counting",
    "hr_semaphore_create_binary",
    "hr_semaphore_take",
    "hr_semaphore_give",
    "hr_semaphore_give_from_isr",
    "HR_TASK_WAIT_SEMAPHORE_TAKE",
]:
    if token not in semaphore_text and token not in task_text:
        print(f"Phase 10 semaphore token missing: {token}")
        sys.exit(1)

for token in [
    "hr_mutex_lock",
    "hr_mutex_unlock",
    "hr_mutex_attach_owner",
    "hr_mutex_detach_owner",
    "hr_mutex_recompute_task_priority",
    "hr_mutex_wait_cleanup",
    "owned_mutexes",
    "effective_priority",
    "HR_TASK_WAIT_MUTEX_LOCK",
]:
    if token not in mutex_text and token not in task_text:
        print(f"Phase 10 mutex token missing: {token}")
        sys.exit(1)

for token in [
    "hr_kernel_block_current_on_wait_list_ex",
    "hr_kernel_set_task_effective_priority",
    "wait_cleanup",
    "HR_ERROR_TIMEOUT",
]:
    if token not in kernel_text:
        print(f"Phase 10 kernel token missing: {token}")
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

    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = shutil.which("llvm-objdump") or shutil.which("arm-none-eabi-objdump")

    images = {
        "10-01-semaphore-from-isr": [
            "EXTI0_IRQHandler",
            "hr_semaphore_take",
            "hr_semaphore_give_from_isr",
        ],
        "10-02-mutex-priority-inheritance": [
            "hr_mutex_lock",
            "hr_mutex_unlock",
            "hr_mutex_recompute_owner_priority",
            "hr_kernel_set_task_effective_priority",
        ],
    }

    for example, required_symbols in images.items():
        elf = ROOT / f"build/{example}/hairtos_baremetal.elf"
        if not elf.is_file():
            # The build loop cleans each image. Rebuild the image being inspected.
            subprocess.run(["make", "clean"], cwd=ROOT, check=True)
            subprocess.run(build_prefix + [f"EXAMPLE={example}", "elf"],
                           cwd=ROOT, check=True)

        if nm_tool:
            symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
            for symbol in required_symbols + ["SysTick_Handler", "PendSV_Handler"]:
                if symbol not in symbols:
                    print(f"missing target symbol in {example}: {symbol}")
                    sys.exit(1)
            if example == "10-01-semaphore-from-isr":
                lines = [line for line in symbols.splitlines()
                         if "EXTI0_IRQHandler" in line]
                if not lines or not any(re.search(r"\bT\b", line) for line in lines):
                    print("EXTI0_IRQHandler is not a strong text symbol")
                    sys.exit(1)

        if objdump_tool:
            disassembly = subprocess.check_output([objdump_tool, "-d", str(elf)], text=True).lower()
            for symbol in required_symbols:
                if symbol.lower() not in disassembly:
                    print(f"Phase 10 disassembly missing in {example}: {symbol}")
                    sys.exit(1)

    # Validate the CMake/Clang build path for both Phase 10 target images.
    if shutil.which("cmake") and shutil.which("ninja") and shutil.which("clang"):
        for example in images:
            build_dir = ROOT / f"build/cmake-{example}"
            subprocess.run(["cmake", "-S", ".", "-B", str(build_dir),
                            "-G", "Ninja",
                            f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake/toolchains/arm-none-eabi-clang.cmake'}",
                            f"-DHAIRTOS_EXAMPLE={example}"],
                           cwd=ROOT, check=True)
            subprocess.run(["cmake", "--build", str(build_dir)],
                           cwd=ROOT, check=True)

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

print("HairRTOS Phase 10 structure check: PASS")
print("HairRTOS Phase 10 host tests: PASS")
if built_target:
    print("HairRTOS Phase 1-9 target regression builds: PASS")
    print("HairRTOS Phase 10 semaphore/mutex target builds: PASS")
    print("HairRTOS Phase 10 symbols/disassembly and CMake builds: PASS")
print("Physical STM32F103 synchronization runtime validation: required on target hardware")

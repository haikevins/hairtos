#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "labs/memory-allocator/include/hr_heap_lab.h",
    "labs/memory-allocator/include/hr_pool_lab.h",
    "labs/memory-allocator/src/hr_heap_lab.c",
    "labs/memory-allocator/src/hr_pool_lab.c",
    "labs/memory-allocator/tests/test_heap_lab.c",
    "labs/memory-allocator/demo.c",
    "labs/memory-allocator/README.md",
    "examples/14-memory-allocator-lab/main.c",
    "examples/14-memory-allocator-lab/README.md",
    "docs/memory-allocator-lab.md",
    "docs/phase14-memory-allocator-lab.md",
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

combined = "\n".join((ROOT / item).read_text(encoding="utf-8")
                     for item in required if item.endswith((".c", ".h")))
for token in [
    "hr_heap_lab_init",
    "hr_heap_lab_alloc",
    "hr_heap_lab_free",
    "hr_heap_lab_validate",
    "hr_heap_lab_get_stats",
    "hr_pool_lab_init",
    "hr_pool_lab_alloc",
    "hr_pool_lab_free",
    "internal_fragmentation_bytes",
    "external_fragmentation_bytes",
    "HR_HEAP_LAB_ERROR_DOUBLE_FREE",
]:
    if token not in combined:
        print(f"Phase 14 token missing: {token}")
        sys.exit(1)

for forbidden in ["malloc(", "calloc(", "realloc("]:
    sources = "\n".join(
        (ROOT / item).read_text(encoding="utf-8")
        for item in required if item.startswith("labs/memory-allocator/src/")
    )
    if forbidden in sources:
        print(f"Allocator lab must not use libc allocation: {forbidden}")
        sys.exit(1)

example_source = (ROOT / "examples/14-memory-allocator-lab/main.c").read_text(
    encoding="utf-8")
if "placeholder" in example_source.lower() or "not implemented" in example_source.lower():
    print("Phase 14 target example is still a placeholder")
    sys.exit(1)
for token in ["hr_heap_lab_alloc", "hr_heap_lab_free", "hr_pool_lab_alloc",
              "internal_frag", "external_frag"]:
    if token not in example_source:
        print(f"Phase 14 target example token missing: {token}")
        sys.exit(1)

host_test = (ROOT / "labs/memory-allocator/tests/test_heap_lab.c").read_text(
    encoding="utf-8")
for token in [
    "test_heap_forward_and_backward_coalescing",
    "test_heap_reports_internal_and_external_fragmentation",
    "test_heap_rejects_invalid_and_double_free",
    "test_heap_randomized_allocate_free_sequences",
    "test_pool_exhaustion_stats_and_recovery",
]:
    if token not in host_test:
        print(f"Phase 14 host-test token missing: {token}")
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
    subprocess.run(["make", f"HOST_CC={host_cc}", "phase14-lab"],
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
    "12-software-timer",
    "13-01-event-post",
    "13-02-active-object",
    "13-03-flat-state-machine",
    "13-04-time-event",
    "13-05-publish-subscribe",
    "13-06-event-driven-demo",
    "14-memory-allocator-lab",
]

built_target = False
if shutil.which("clang") and shutil.which("ld.lld") and shutil.which("llvm-objcopy"):
    build_prefix = ["make", "-s", "TOOLCHAIN=clang"]
    built_target = True
elif shutil.which("arm-none-eabi-gcc") and shutil.which("arm-none-eabi-objcopy"):
    build_prefix = ["make", "-s"]
    built_target = True
else:
    build_prefix = []
    print("Target builds skipped: no complete ARM cross toolchain was found")

if built_target:
    for example in examples:
        subprocess.run(["make", "clean"], cwd=ROOT, check=True,
                       stdout=subprocess.DEVNULL)
        subprocess.run(build_prefix + [f"EXAMPLE={example}", "all"],
                       cwd=ROOT, check=True)

    subprocess.run(["make", "clean"], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    subprocess.run(build_prefix + ["EXAMPLE=14-memory-allocator-lab", "elf"],
                   cwd=ROOT, check=True)
    elf = ROOT / "build/14-memory-allocator-lab/hairtos_baremetal.elf"

    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = (shutil.which("llvm-objdump") or
                    shutil.which("arm-none-eabi-objdump"))
    required_symbols = [
        "hr_heap_lab_init",
        "hr_heap_lab_alloc",
        "hr_heap_lab_free",
        "hr_heap_lab_validate",
        "hr_heap_lab_get_stats",
        "hr_pool_lab_init",
        "hr_pool_lab_alloc",
        "hr_pool_lab_free",
    ]

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        for symbol in required_symbols:
            if symbol not in symbols:
                print(f"missing Phase 14 target symbol: {symbol}")
                sys.exit(1)
        for forbidden in ["malloc", "calloc", "realloc"]:
            if re.search(rf"\b{forbidden}\b", symbols):
                print(f"unexpected libc allocator symbol in target: {forbidden}")
                sys.exit(1)

    if objdump_tool:
        disassembly = subprocess.check_output(
            [objdump_tool, "-d", str(elf)], text=True).lower()
        for symbol in required_symbols:
            if symbol.lower() not in disassembly:
                print(f"Phase 14 disassembly missing: {symbol}")
                sys.exit(1)

    if shutil.which("cmake") and shutil.which("ninja") and shutil.which("clang"):
        build_dir = ROOT / "build/cmake-14-memory-allocator-lab"
        subprocess.run([
            "cmake", "-S", ".", "-B", str(build_dir), "-G", "Ninja",
            f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake/toolchains/arm-none-eabi-clang.cmake'}",
            "-DHAIRTOS_EXAMPLE=14-memory-allocator-lab",
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
    sys.exit(1)

print("HairRTOS Phase 14 check: PASS")
print("Fixed-block pool and first-fit heap lab validated")

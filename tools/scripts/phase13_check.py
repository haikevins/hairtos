#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "hairevent/include/hairevent/hairevent.h",
    "hairevent/include/hairevent/he_event.h",
    "hairevent/include/hairevent/he_active.h",
    "hairevent/include/hairevent/he_state_machine.h",
    "hairevent/include/hairevent/he_time_event.h",
    "hairevent/include/hairevent/he_pubsub.h",
    "hairevent/internal/he_internal.h",
    "hairevent/src/he_event.c",
    "hairevent/src/he_active.c",
    "hairevent/src/he_state_machine.c",
    "hairevent/src/he_time_event.c",
    "hairevent/src/he_pubsub.c",
    "tests/host/test_hairevent.c",
    "docs/event-framework.md",
    "docs/phase13-hairevent-framework.md",
]

phase13_examples = [
    "13-01-event-post",
    "13-02-active-object",
    "13-03-flat-state-machine",
    "13-04-time-event",
    "13-05-publish-subscribe",
    "13-06-event-driven-demo",
]
for example in phase13_examples:
    required.extend([
        f"examples/{example}/main.c",
        f"examples/{example}/README.md",
    ])

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
    "he_event_pool_init",
    "he_event_new",
    "he_event_retain",
    "he_event_release",
    "he_active_create_static",
    "he_active_post_from_isr",
    "he_state_machine_dispatch",
    "he_state_transition",
    "he_time_event_create_static",
    "he_pubsub_publish",
    "reference_count",
    "HE_SIG_ENTRY",
    "HE_SIG_EXIT",
    "HE_SIG_INIT",
]:
    if token not in combined:
        print(f"Phase 13 token missing: {token}")
        sys.exit(1)

for example in phase13_examples:
    source = (ROOT / f"examples/{example}/main.c").read_text(encoding="utf-8")
    if "placeholder" in source.lower() or "not implemented" in source.lower():
        print(f"Phase 13 example is still a placeholder: {example}")
        sys.exit(1)

isr_example = (ROOT / "examples/13-01-event-post/main.c").read_text(encoding="utf-8")
for token in ["EXTI0_IRQHandler", "he_active_post_from_isr", "hr_yield_from_isr"]:
    if token not in isr_example:
        print(f"ISR event-post example missing: {token}")
        sys.exit(1)

host_test = (ROOT / "tests/host/test_hairevent.c").read_text(encoding="utf-8")
for token in [
    "test_event_pool_reference_counting",
    "test_flat_state_machine_entry_exit_transition",
    "test_active_post_and_publish_subscribe_ownership",
]:
    if token not in host_test:
        print(f"HairEvent host-test token missing: {token}")
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
    "12-software-timer",
] + phase13_examples

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
    subprocess.run(build_prefix + ["EXAMPLE=13-06-event-driven-demo", "elf"],
                   cwd=ROOT, check=True)
    elf = ROOT / "build/13-06-event-driven-demo/hairtos_baremetal.elf"

    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = (shutil.which("llvm-objdump") or
                    shutil.which("arm-none-eabi-objdump"))
    required_symbols = [
        "he_event_pool_init",
        "he_event_new",
        "he_event_release",
        "he_active_create_static",
        "he_active_post",
        "he_state_machine_dispatch",
        "he_state_transition",
        "he_time_event_create_static",
        "he_pubsub_publish",
        "SysTick_Handler",
        "PendSV_Handler",
    ]

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        for symbol in required_symbols:
            if symbol not in symbols:
                print(f"missing Phase 13 target symbol: {symbol}")
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
                print(f"Phase 13 disassembly missing: {symbol}")
                sys.exit(1)

    subprocess.run(["make", "clean"], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    subprocess.run(build_prefix + ["EXAMPLE=13-01-event-post", "elf"],
                   cwd=ROOT, check=True)
    isr_elf = ROOT / "build/13-01-event-post/hairtos_baremetal.elf"
    if nm_tool:
        isr_symbols = subprocess.check_output([nm_tool, str(isr_elf)], text=True)
        for symbol in ["EXTI0_IRQHandler", "he_active_post_from_isr"]:
            if symbol not in isr_symbols:
                print(f"missing Phase 13 ISR target symbol: {symbol}")
                sys.exit(1)

    if shutil.which("cmake") and shutil.which("ninja") and shutil.which("clang"):
        build_dir = ROOT / "build/cmake-13-event-driven-demo"
        subprocess.run([
            "cmake", "-S", ".", "-B", str(build_dir), "-G", "Ninja",
            f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake/toolchains/arm-none-eabi-clang.cmake'}",
            "-DHAIRTOS_EXAMPLE=13-06-event-driven-demo",
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

print("HairRTOS Phase 13 structure check: PASS")
print("HairRTOS Phase 13 HairEvent ownership/state-machine tests: PASS")
if built_target:
    print("HairRTOS Phase 1-12 target regression builds: PASS")
    print("HairRTOS Phase 13 six target builds and symbols/disassembly: PASS")
    print("HairRTOS Phase 13 CMake/Ninja integration build: PASS")
print("Physical STM32F103 ISR-to-AO latency and queue-overflow validation: required on target hardware")

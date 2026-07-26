#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "benchmarks/kernel/include/hr_benchmark.h",
    "benchmarks/kernel/src/hr_benchmark_stats.c",
    "benchmarks/kernel/src/hr_benchmark_dwt.c",
    "benchmarks/kernel/src/hr_benchmark_gpio.c",
    "benchmarks/kernel/README.md",
    "tests/host/test_benchmark.c",
    "tests/benchmark/README.md",
    "examples/15-kernel-benchmark/main.c",
    "examples/15-kernel-benchmark/README.md",
    "docs/benchmark-plan.md",
    "docs/kernel-benchmark.md",
    "docs/phase15-kernel-benchmark.md",
    ".vscode/c_cpp_properties.json",
]

missing = [item for item in required if not (ROOT / item).is_file()]
empty = [item for item in required
         if (ROOT / item).is_file() and (ROOT / item).stat().st_size == 0]
if missing or empty:
    for item in missing:
        print(f"missing: {item}")
    for item in empty:
        print(f"empty: {item}")
    sys.exit(1)

phase15_text = "\n".join((ROOT / item).read_text(encoding="utf-8")
                           for item in required if item.endswith((".c", ".h", ".md")))
for forbidden in ["Planning placeholder", "Roadmap placeholder", "not implemented"]:
    if forbidden.lower() in phase15_text.lower():
        print(f"Phase 15 still contains placeholder text: {forbidden}")
        sys.exit(1)

for token in [
    "hr_benchmark_clock_init",
    "hr_benchmark_clock_now",
    "hr_benchmark_stats_record",
    "hr_benchmark_stats_percentile",
    "hr_benchmark_elapsed_cycles",
    "hr_benchmark_adjust_cycles",
    "HR_BENCHMARK_DWT_CYCCNT_ADDRESS",
    "hr_benchmark_gpio_mark_begin",
    "benchmark_measure_yield_roundtrip",
    "benchmark_measure_queue_wakeup",
    "benchmark_measure_event_dispatch",
    "benchmark_measure_timer_jitter",
    "measurement_overhead_cycles",
    "metric,count,min,p50,mean,p95,max,mean_ns",
]:
    if token not in phase15_text:
        print(f"Phase 15 token missing: {token}")
        sys.exit(1)

main_source = (ROOT / "examples/15-kernel-benchmark/main.c").read_text(
    encoding="utf-8")
for token in [
    "hr_scheduler_select_highest",
    "hr_queue_send",
    "hr_semaphore_take",
    "hr_mutex_lock",
    "hr_timer_start",
    "he_active_post",
    "hr_task_get_stack_high_watermark",
    "_sidata",
    "_ebss",
]:
    if token not in main_source:
        print(f"Phase 15 benchmark path missing: {token}")
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
    subprocess.run(["make", "clean"], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    subprocess.run(["make", "-s", f"HOST_CC={host_cc}", "host-tests"],
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
    "15-kernel-benchmark",
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
    subprocess.run(build_prefix + ["EXAMPLE=15-kernel-benchmark", "elf"],
                   cwd=ROOT, check=True)
    elf = ROOT / "build/15-kernel-benchmark/hairtos_baremetal.elf"

    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = (shutil.which("llvm-objdump") or
                    shutil.which("arm-none-eabi-objdump"))
    size_tool = shutil.which("size") or shutil.which("arm-none-eabi-size")
    required_symbols = [
        "hr_benchmark_clock_init",
        "hr_benchmark_clock_now",
        "hr_benchmark_stats_percentile",
        "hr_benchmark_gpio_mark_begin",
        "hr_scheduler_select_highest",
        "hr_queue_send",
        "hr_semaphore_take",
        "hr_mutex_lock",
        "hr_timer_start",
        "he_active_post",
        "SVC_Handler",
        "PendSV_Handler",
        "SysTick_Handler",
    ]

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        for symbol in required_symbols:
            if symbol not in symbols:
                print(f"missing Phase 15 target symbol: {symbol}")
                sys.exit(1)
        if "__aeabi_uldivmod" in symbols:
            print("Phase 15 unexpectedly depends on 64-bit division runtime support")
            sys.exit(1)

    if objdump_tool:
        disassembly = subprocess.check_output(
            [objdump_tool, "-d", str(elf)], text=True).lower()
        for symbol in required_symbols:
            if symbol.lower() not in disassembly:
                print(f"Phase 15 disassembly missing: {symbol}")
                sys.exit(1)
        for instruction in ["mrs", "msr", "svc"]:
            if instruction not in disassembly:
                print(f"Phase 15 exception/context instruction missing: {instruction}")
                sys.exit(1)

    if size_tool:
        output = subprocess.check_output([size_tool, str(elf)], text=True)
        match = re.search(r"\n\s*(\d+)\s+(\d+)\s+(\d+)\s+\d+\s+[0-9a-fA-F]+\s+", output)
        if not match:
            print("Could not parse Phase 15 image size")
            print(output)
            sys.exit(1)
        text_bytes, data_bytes, bss_bytes = map(int, match.groups())
        if (text_bytes + data_bytes) > (64 * 1024):
            print("Phase 15 Flash image exceeds 64 KiB")
            sys.exit(1)
        if (data_bytes + bss_bytes + 1024) > (20 * 1024):
            print("Phase 15 static RAM plus reserved MSP stack exceeds 20 KiB")
            sys.exit(1)
        print(f"Phase 15 image: text={text_bytes}, data={data_bytes}, bss={bss_bytes}")

    # Verify ordinary examples do not link benchmark support.
    subprocess.run(["make", "clean"], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    subprocess.run(build_prefix + ["EXAMPLE=14-memory-allocator-lab", "elf"],
                   cwd=ROOT, check=True)
    normal_elf = ROOT / "build/14-memory-allocator-lab/hairtos_baremetal.elf"
    if nm_tool:
        normal_symbols = subprocess.check_output([nm_tool, str(normal_elf)], text=True)
        if "hr_benchmark_clock_init" in normal_symbols:
            print("Benchmark support leaked into a normal target image")
            sys.exit(1)

    if shutil.which("cmake") and shutil.which("ninja") and shutil.which("clang"):
        build_dir = ROOT / "build/cmake-15-kernel-benchmark"
        subprocess.run([
            "cmake", "-S", ".", "-B", str(build_dir), "-G", "Ninja",
            f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake/toolchains/arm-none-eabi-clang.cmake'}",
            "-DHAIRTOS_EXAMPLE=15-kernel-benchmark",
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

print("HairRTOS Phase 15 check: PASS")
print("HairRTOS benchmark statistics tests: PASS")
if built_target:
    print("HairRTOS Phase 1-14 target regression builds: PASS")
    print("HairRTOS Phase 15 Make/CMake target and symbol/disassembly checks: PASS")
print("Physical DWT/GPIO benchmark values must be collected on STM32F103 hardware")

#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "VERSION",
    "kernel/include/hairtos/hr_diagnostics.h",
    "kernel/include/hairtos/hr_hooks.h",
    "kernel/internal/hr_diagnostics_internal.h",
    "kernel/src/hr_diagnostics.c",
    "arch/arm/cortex-m3/hr_fault.c",
    "arch/arm/cortex-m3/hr_faultasm.S",
    "tests/host/test_diagnostics.c",
    "tests/stress/phase16_stress.h",
    "tests/stress/phase16_stress_core.c",
    "tests/stress/test_phase16_stress.c",
    "tests/stress/phase16_stress_main.c",
    "tests/portability/cortex_m0_compile_probe.c",
    "tools/scripts/cortex_m0_proof.py",
    "examples/16-diagnostics-stress-stabilization/main.c",
    "examples/16-diagnostics-stress-stabilization/README.md",
    "docs/diagnostics.md",
    "docs/stress-test-plan.md",
    "docs/api-reference.md",
    "docs/porting-guide.md",
    "docs/cortex-m0-port-proof.md",
    "docs/release-checklist.md",
    "docs/phase16-diagnostics-stabilization.md",
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

if (ROOT / "framework/event").exists():
    print("Obsolete framework/event placeholder tree still exists")
    sys.exit(1)

phase16_text = "\n".join(
    (ROOT / item).read_text(encoding="utf-8")
    for item in required
    if item.endswith((".c", ".h", ".S", ".md"))
)
for forbidden in ["Planning placeholder", "Roadmap placeholder", "not implemented"]:
    if forbidden.lower() in phase16_text.lower():
        print(f"Phase 16 still contains placeholder text: {forbidden}")
        sys.exit(1)

for token in [
    "hr_diagnostics_run_health_check",
    "hr_diagnostics_get_runtime_statistics",
    "hr_diagnostics_record_fault",
    "hr_kernel_validate_internal",
    "HR_ASSERT",
    ".noinit.hairtos",
    "HardFault_Handler",
    "UsageFault_Handler",
    "phase16_scheduler_stress_run",
    "HR_PHASE16_INJECT_USAGE_FAULT",
    "500000",
]:
    if token not in phase16_text and token not in (
        ROOT / "boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld"
    ).read_text(encoding="utf-8"):
        print(f"Phase 16 token missing: {token}")
        sys.exit(1)

subprocess.run(["python3", "tools/scripts/phase0_check.py"], cwd=ROOT, check=True)
subprocess.run(["python3", "tools/scripts/example_layout_check.py"], cwd=ROOT, check=True)
subprocess.run(["python3", "tools/scripts/roadmap_check.py"], cwd=ROOT, check=True)
subprocess.run(["python3", "tools/scripts/cortex_m0_proof.py"], cwd=ROOT, check=True)

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

subprocess.run(["make", "clean"], cwd=ROOT, check=True,
               stdout=subprocess.DEVNULL)
subprocess.run(["make", "-s", "phase16-stress"], cwd=ROOT, check=True)

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
    "16-diagnostics-stress-stabilization",
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
    subprocess.run(build_prefix + [
        "EXAMPLE=16-diagnostics-stress-stabilization", "elf"
    ], cwd=ROOT, check=True)
    elf = ROOT / "build/16-diagnostics-stress-stabilization/hairtos_baremetal.elf"

    nm_tool = (shutil.which("llvm-nm") or shutil.which("arm-none-eabi-nm") or
               shutil.which("nm"))
    objdump_tool = (shutil.which("llvm-objdump") or
                    shutil.which("arm-none-eabi-objdump"))
    readelf_tool = (shutil.which("llvm-readelf") or
                    shutil.which("arm-none-eabi-readelf") or
                    shutil.which("readelf"))
    size_tool = shutil.which("size") or shutil.which("arm-none-eabi-size")
    required_symbols = [
        "hr_diagnostics_initialize",
        "hr_diagnostics_run_health_check",
        "hr_diagnostics_get_runtime_statistics",
        "hr_diagnostics_record_fault",
        "hr_kernel_validate_internal",
        "hr_port_fault_capture",
        "NMI_Handler",
        "HardFault_Handler",
        "MemManage_Handler",
        "BusFault_Handler",
        "UsageFault_Handler",
        "SVC_Handler",
        "PendSV_Handler",
        "SysTick_Handler",
    ]

    if nm_tool:
        symbols = subprocess.check_output([nm_tool, str(elf)], text=True)
        for symbol in required_symbols:
            if symbol not in symbols:
                print(f"missing Phase 16 target symbol: {symbol}")
                sys.exit(1)
        for handler in ["NMI_Handler", "HardFault_Handler", "MemManage_Handler",
                        "BusFault_Handler", "UsageFault_Handler"]:
            if not re.search(rf"\b[0-9a-fA-F]+\s+[Tt]\s+{handler}\b", symbols):
                print(f"fault handler is not a strong text symbol: {handler}")
                sys.exit(1)
        if "__aeabi_memcpy" in symbols or "__aeabi_memclr" in symbols:
            print("Phase 16 unexpectedly depends on freestanding memcpy/memclr support")
            sys.exit(1)

    if objdump_tool:
        disassembly = subprocess.check_output(
            [objdump_tool, "-d", str(elf)], text=True).lower()
        for token in ["hr_port_fault_capture", "hardfault_handler",
                      "usagefault_handler", "mrs", "msp", "psp", "svc"]:
            if token not in disassembly:
                print(f"Phase 16 disassembly missing: {token}")
                sys.exit(1)

    if readelf_tool:
        sections = subprocess.check_output(
            [readelf_tool, "-S", str(elf)], text=True)
        if ".noinit" not in sections:
            print("Phase 16 ELF does not contain the retained .noinit section")
            sys.exit(1)

    if size_tool:
        output = subprocess.check_output([size_tool, str(elf)], text=True)
        match = re.search(r"\n\s*(\d+)\s+(\d+)\s+(\d+)\s+\d+\s+[0-9a-fA-F]+\s+", output)
        if not match:
            print("Could not parse Phase 16 image size")
            print(output)
            sys.exit(1)
        text_bytes, data_bytes, bss_bytes = map(int, match.groups())
        if (text_bytes + data_bytes) > (64 * 1024):
            print("Phase 16 Flash image exceeds 64 KiB")
            sys.exit(1)
        if (data_bytes + bss_bytes + 1024) > (20 * 1024):
            print("Phase 16 static RAM plus reserved MSP stack exceeds 20 KiB")
            sys.exit(1)
        print(f"Phase 16 image: text={text_bytes}, data={data_bytes}, bss={bss_bytes}")

    # Compile the optional fault-injection path without running it.
    subprocess.run(["make", "clean"], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    subprocess.run(build_prefix + [
        "EXAMPLE=16-diagnostics-stress-stabilization",
        "EXTRA_DEFINES=-DHR_PHASE16_INJECT_USAGE_FAULT=1",
        "elf",
    ], cwd=ROOT, check=True)

    # Diagnostics must remain absent from ordinary target images.
    subprocess.run(["make", "clean"], cwd=ROOT, check=True,
                   stdout=subprocess.DEVNULL)
    subprocess.run(build_prefix + ["EXAMPLE=14-memory-allocator-lab", "elf"],
                   cwd=ROOT, check=True)
    normal_elf = ROOT / "build/14-memory-allocator-lab/hairtos_baremetal.elf"
    if nm_tool:
        normal_symbols = subprocess.check_output([nm_tool, str(normal_elf)], text=True)
        if "hr_diagnostics_run_health_check" in normal_symbols:
            print("Diagnostics support leaked into a normal target image")
            sys.exit(1)

    if shutil.which("cmake") and shutil.which("ninja") and shutil.which("clang"):
        build_dir = ROOT / "build/cmake-16-diagnostics"
        subprocess.run([
            "cmake", "-S", ".", "-B", str(build_dir), "-G", "Ninja",
            f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake/toolchains/arm-none-eabi-clang.cmake'}",
            "-DHAIRTOS_EXAMPLE=16-diagnostics-stress-stabilization",
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

print("HairRTOS Phase 16 check: PASS")
print("HairRTOS host diagnostics and 500,000-operation stress: PASS")
if built_target:
    print("HairRTOS Phase 1-16 target regression builds: PASS")
    print("HairRTOS fault symbols, .noinit, Make/CMake, and size checks: PASS")
print("Physical eight-hour stress and injected-fault reset verification remain pending")

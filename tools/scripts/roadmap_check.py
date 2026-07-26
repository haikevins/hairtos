#!/usr/bin/env python3

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "docs/roadmap.md",
    "docs/requirements-coverage.md",
    "docs/task-suspend-resume.md",
    "docs/software-timer.md",
    "docs/phase12-software-timer.md",
    "docs/phase13-hairevent-framework.md",
    "docs/phase14-memory-allocator-lab.md",
    "docs/phase15-kernel-benchmark.md",
    "docs/kernel-benchmark.md",
    "docs/event-framework.md",
    "docs/memory-allocator-lab.md",
    "docs/benchmark-plan.md",
    "docs/example-index.md",
    "labs/memory-allocator/README.md",
    "benchmarks/kernel/README.md",
]

missing = [item for item in required if not (ROOT / item).is_file()]
if missing:
    for item in missing:
        print(f"missing: {item}")
    sys.exit(1)

roadmap = (ROOT / "docs/roadmap.md").read_text(encoding="utf-8")
required_phases = [
    "Phase 1 — Bare-metal foundation",
    "Phase 2 — Intrusive lists and kernel data structures",
    "Phase 11 — Task suspend/resume",
    "Phase 12 — Software timer service",
    "Phase 13 — HairEvent framework",
    "Phase 14 — Memory allocator lab",
    "Phase 15 — Kernel benchmarks",
    "Phase 16 — Diagnostics, stress, and stabilization",
]
missing_phases = [item for item in required_phases if item not in roadmap]
if missing_phases:
    for item in missing_phases:
        print(f"roadmap entry missing: {item}")
    sys.exit(1)

readme = (ROOT / "README.md").read_text(encoding="utf-8")
if "Current status: Phase 15 complete" not in readme:
    print("README no longer identifies Phase 15 as the current completed phase")
    sys.exit(1)

subprocess_required = ROOT / "tools/scripts/example_layout_check.py"
if not subprocess_required.is_file():
    print("missing example layout checker")
    sys.exit(1)

print("HairRTOS roadmap check: PASS")
print("Current implementation phase: Phase 15")
print("Future roadmap addition: diagnostics and stabilization")

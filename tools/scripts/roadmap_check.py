#!/usr/bin/env python3

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]

required = [
    "docs/roadmap.md",
    "docs/requirements-coverage.md",
    "docs/task-suspend-resume.md",
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
if "Current status: Phase 4 complete" not in readme:
    print("README no longer identifies Phase 4 as the current completed phase")
    sys.exit(1)

subprocess_required = ROOT / "tools/scripts/example_layout_check.py"
if not subprocess_required.is_file():
    print("missing example layout checker")
    sys.exit(1)

print("HairRTOS roadmap check: PASS")
print("Current implementation phase: Phase 4")
print("Future roadmap additions: suspend/resume, allocator lab, benchmarks")

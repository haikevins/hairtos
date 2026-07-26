#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
EXAMPLES = ROOT / "examples"
EXPECTED = ['01-baremetal-foundation', '02-kernel-data-structures-host', '03-static-task-stack', '04-start-first-task', '05-cooperative-context-switch', '06-priority-scheduler', '07-task-delay-timeout', '08-preemption-round-robin', '09-queue-blocking-ipc', '10-01-semaphore-from-isr', '10-02-mutex-priority-inheritance', '11-task-suspend-resume', '12-software-timer', '13-01-event-post', '13-02-active-object', '13-03-flat-state-machine', '13-04-time-event', '13-05-publish-subscribe', '13-06-event-driven-demo', '14-memory-allocator-lab', '15-kernel-benchmark', '16-diagnostics-stress-stabilization']
FORBIDDEN = ['00-baremetal-blink', '01-static-task-stack', '01-start-first-task', '02-cooperative-switch', '03-preemptive-priority', '05-task-delay', '04-round-robin', '06-queue', '07-semaphore-from-isr', '08-mutex-priority-inheritance', '16-task-suspend-resume', '09-software-timer', '10-event-post', '11-active-object', '12-flat-state-machine', '13-time-event', '14-publish-subscribe', '15-event-driven-demo', '17-memory-allocator-lab', '18-kernel-benchmark']

actual = sorted(path.name for path in EXAMPLES.iterdir() if path.is_dir())

README = EXAMPLES / "README.md"
if not README.is_file():
    print("Missing examples/README.md")
    sys.exit(1)

readme_text = README.read_text(encoding="utf-8")
missing_from_readme = [name for name in EXPECTED if name not in readme_text]
if missing_from_readme:
    print("Examples missing from examples/README.md:")
    for name in missing_from_readme:
        print("  - " + name)
    sys.exit(1)
missing = [name for name in EXPECTED if name not in actual]
extra = [name for name in actual if name not in EXPECTED]
forbidden = [name for name in FORBIDDEN if (EXAMPLES / name).exists()]

def fail(label, items):
    if items:
        print(label + ":")
        for item in items:
            print("  - " + item)

if missing or extra or forbidden:
    fail("Missing canonical examples", missing)
    fail("Unexpected examples", extra)
    fail("Legacy example names still present", forbidden)
    sys.exit(1)

prefixes = {}
for name in actual:
    prefix = name.split("-", 1)[0]
    prefixes.setdefault(prefix, []).append(name)

# Exact duplicate full names are impossible in a directory, but the check also
# documents that Phase 10 and 13 intentionally use sub-numbered examples.
print("HairRTOS example layout check: PASS")
print(f"Canonical example directories: {len(actual)}")
print("Implemented examples: Phase 1, Phase 2 host, Phase 3, Phase 4, Phase 5, Phase 6, Phase 7")

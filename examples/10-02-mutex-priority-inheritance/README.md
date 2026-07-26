# Phase 10.2 — Mutex and Priority Inheritance

This target example reproduces classic priority inversion:

- High priority: 1
- Medium priority: 3
- Low priority: 5

The low-priority task acquires a mutex first. High later blocks on that mutex.
hairtos temporarily changes Low's effective priority from 5 to 1, so the
medium-priority CPU-bound task cannot prevent Low from finishing the critical
section. Unlock directly transfers ownership to High and restores Low to its
base priority.

Build and flash:

```bash
make EXAMPLE=10-02-mutex-priority-inheritance run
```

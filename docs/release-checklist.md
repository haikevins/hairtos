# HairRTOS v1.0 Release Checklist

- [x] Roadmap Phase 0–16 source implementation complete.
- [x] Static-first kernel; allocator lab remains isolated.
- [x] Clang and GCC host tests run with ASan/UBSan.
- [x] Deterministic 500,000-operation native scheduler stress passes.
- [x] Every target example cross-builds without warnings.
- [x] Make and CMake/Ninja build the Phase 16 target.
- [x] SVC, PendSV, SysTick, and strong fault handlers exist in the image.
- [x] Panic record is placed in `.noinit`.
- [x] Cortex-M0 architecture-independent compile proof passes.
- [x] Public API context/blocking rules are documented.
- [ ] Flash and run all examples on a physical STM32F103C8T6.
- [ ] Run the Phase 16 target for at least eight hours.
- [ ] Verify retained UsageFault record using the optional fault injection.
- [ ] Record board/toolchain revisions and tag `v1.0.0` after hardware sign-off.

The unchecked hardware items are intentionally not claimed by the archive.

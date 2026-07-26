# Release checklist

## 1. Source

- [ ] Không placeholder hoặc unused skeleton.
- [ ] Public/internal boundary đúng.
- [ ] Make và CMake source list đồng bộ.
- [ ] Version/changelog/roadmap đồng bộ.

## 2. Build

- [ ] Host Clang sanitizer PASS.
- [ ] Host GCC sanitizer PASS.
- [ ] Tất cả target example build.
- [ ] CMake/Ninja build example `hairtos`.
- [ ] No warning với `-Werror`.

## 3. Runtime

- [ ] Flash Blue Pill thật.
- [ ] UART log expected cho Phase 4–16.
- [ ] Preemption/round-robin quan sát được.
- [ ] ISR semaphore path PASS.
- [ ] Fault injection giữ record qua reset.
- [ ] Stress dài hạn PASS.

## 4. Resources

- [ ] Flash/RAM trong linker limit.
- [ ] Stack margins đủ.
- [ ] Benchmark sample buffers không tràn RAM.
- [ ] Event pool/queue capacities có rationale.

## 5. Package

- [ ] ZIP không có `.git`/build/cache.
- [ ] SHA-256 được tạo và verify.
- [ ] Docs link check PASS.

## 6. Known limitations

Liệt kê rõ phần chưa test trên hardware, toolchain chưa có, API chưa hỗ trợ và board assumptions.

# Testing guide

## 1. Mục tiêu

Phân biệt host unit test, cross-build regression và hardware runtime validation.

## 2. Host tests

```bash
make host-tests
```

Build native với ASan/UBSan, kiểm tra intrusive lists, scheduler, TCB stack, queue, semaphore, mutex, timers, haievent, allocator, benchmark stats và diagnostics.

Chạy compiler khác:

```bash
make HOST_CC=clang host-tests
make HOST_CC=gcc host-tests
```

## 3. Phase checks

```bash
make EXAMPLE=16-diagnostics-stress-stabilization check
```

Script phase cuối chạy hygiene, host regression, stress, target builds, symbol/disassembly checks, CMake/Ninja và memory limits.

## 4. Target build

```bash
make EXAMPLE=16-diagnostics-stress-stabilization TOOLCHAIN=clang build
```

Cross-build PASS chỉ chứng minh compile/link/static checks, không chứng minh behavior trên board.

## 5. Hardware test

- Flash đúng example bằng cùng `EXAMPLE` argument.
- Mở UART 115200 8-N-1.
- Kiểm tra expected sequence.
- Đo PB0/DWT khi benchmark.
- Chạy fault injection và reset.
- Chạy stress dài hạn.

## 6. Test naming

Mỗi bug fix nên có test mô tả behavior, không chỉ test function riêng. Invariant validation được gọi sau operation sequence.

## 7. Kết quả cần lưu

Compiler version, flags, board revision, clock, image size, UART log, duration và fault record.

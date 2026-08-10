# Testing guide

## Host unit tests

```bash
make TARGET=bluepill_f103c8 host-tests
```

Host build dùng mock port và sanitizer để kiểm tra generic C.

Nhóm coverage hiện gồm:

- intrusive list;
- ready set/scheduler policy;
- wait list;
- timeout;
- task/initial stack;
- kernel start/select;
- queue;
- semaphore;
- mutex/priority inheritance;
- software timer;
- haievent;
- allocator;
- benchmark statistics;
- diagnostics;
- scheduler stress.

## Sanitizer

ASan/UBSan là test aid, không phải target runtime. Một số loader environment có thể yêu cầu ASan runtime được load trước shared libraries. Nếu test binary chưa chạy mà báo `ASan runtime does not come first`, xác minh compiler/runtime loader trước khi kết luận source fail.

## Target build

```bash
make TARGET=bluepill_f103c8 \
     TOOLCHAIN=clang \
     EXAMPLE=16-diagnostics-stress-stabilization \
     build
```

Cross-build kiểm tra:

- header/include boundary;
- ARM codegen;
- assembly ABI link;
- linker script;
- duplicate/missing handler symbol;
- image size.

## Target runtime

Runtime test cần:

- flash;
- UART log;
- LED/marker khi relevant;
- expected preemption/order;
- fault injection;
- reset behavior.

## Khi sửa kernel state machine

Thêm regression test cho sequence cụ thể gây lỗi, không chỉ test API happy path.

## Khi sửa port

Chạy examples theo ladder 01→04→05→08→10-01→15→16.

## Khi sửa haievent

Chạy event ownership tests và examples 13-01..13-06.

## Khi sửa allocator

Host sanitizer trước target demo.

## Test result metadata

Khi lưu benchmark/release result, ghi:

```text
commit/version
target
board revision
toolchain version
optimization
clock
config overrides
duration
UART log
image size
```

# Các ví dụ của hairtos

Thư mục `examples/` chứa các bài thực hành theo thứ tự phát triển từ nền tảng bare-metal đến diagnostics/stress tích hợp. Mỗi thư mục có một `main.c` độc lập và một README dùng chung bố cục 10 mục:

1. Mục tiêu học tập.
2. Kiến thức trọng tâm.
3. Thành phần và cấu hình.
4. Luồng thực thi.
5. API và mã nguồn liên quan.
6. Biên dịch, chạy và kiểm tra.
7. Kết quả mong đợi.
8. Tiêu chí PASS và xử lý lỗi.
9. Giới hạn của ví dụ.
10. Liên hệ với lộ trình.

## Quy ước môi trường

- **Host:** chạy chương trình native trên máy phát triển; không flash STM32.
- **Target:** biên dịch chéo và chạy trên STM32F103C8T6 Blue Pill.
- **Host + Target:** phải chọn `ENVIRONMENT=host` hoặc `ENVIRONMENT=target` để chỉ rõ môi trường.
- `run` trên host chạy file thực thi; `run` trên target biên dịch, flash, xác minh và reset qua OpenOCD.

## Lệnh chung

```bash
make help
make list-examples
make EXAMPLE=<name> build
make EXAMPLE=<name> run
make EXAMPLE=<name> check
make EXAMPLE=<name> clean
make host-tests
make clean-all
```

Với example hỗ trợ hai môi trường:

```bash
make ENVIRONMENT=host   EXAMPLE=14-memory-allocator-lab run
make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run
```

## Phần cứng chung cho target

| Thành phần | Kết nối / cấu hình |
|---|---|
| Board | STM32F103C8T6 Blue Pill |
| Debugger | ST-Link V2: SWDIO, SWCLK, GND, 3.3 V reference |
| UART log | USART1 PA9 TX → USB-UART RX, PA10 RX ← USB-UART TX, GND chung |
| Terminal | 115200 baud, 8 data bits, no parity, 1 stop bit |
| LED | PC13, active-low |

## Danh sách ví dụ

| Ví dụ | Môi trường | Nội dung | Lệnh chạy chính |
| --- | --- | --- | --- |
| [`01-baremetal-foundation`](01-baremetal-foundation/README.md) | Target | Nền tảng bare-metal | `make EXAMPLE=01-baremetal-foundation run` |
| [`02-kernel-data-structures-host`](02-kernel-data-structures-host/README.md) | Host | Cấu trúc dữ liệu kernel — Demo trên host | `make EXAMPLE=02-kernel-data-structures-host run` |
| [`03-static-task-stack`](03-static-task-stack/README.md) | Target | TCB tĩnh và ngăn xếp khởi tạo của tác vụ | `make EXAMPLE=03-static-task-stack run` |
| [`04-start-first-task`](04-start-first-task/README.md) | Target | Khởi chạy tác vụ đầu tiên bằng SVC | `make EXAMPLE=04-start-first-task run` |
| [`05-cooperative-context-switch`](05-cooperative-context-switch/README.md) | Target | Chuyển ngữ cảnh hợp tác | `make EXAMPLE=05-cooperative-context-switch run` |
| [`06-priority-scheduler`](06-priority-scheduler/README.md) | Target | Bộ lập lịch ưu tiên cố định | `make EXAMPLE=06-priority-scheduler run` |
| [`07-task-delay-timeout`](07-task-delay-timeout/README.md) | Target | SysTick, trì hoãn tác vụ và timeout | `make EXAMPLE=07-task-delay-timeout run` |
| [`08-preemption-round-robin`](08-preemption-round-robin/README.md) | Target | Chiếm quyền và Round-Robin | `make EXAMPLE=08-preemption-round-robin run` |
| [`09-queue-blocking-ipc`](09-queue-blocking-ipc/README.md) | Target | Queue và IPC chặn | `make EXAMPLE=09-queue-blocking-ipc run` |
| [`10-01-semaphore-from-isr`](10-01-semaphore-from-isr/README.md) | Target | Trao semaphore từ ISR | `make EXAMPLE=10-01-semaphore-from-isr run` |
| [`10-02-mutex-priority-inheritance`](10-02-mutex-priority-inheritance/README.md) | Target | Mutex và kế thừa ưu tiên | `make EXAMPLE=10-02-mutex-priority-inheritance run` |
| [`11-task-suspend-resume`](11-task-suspend-resume/README.md) | Target | Tạm dừng và tiếp tục tác vụ | `make EXAMPLE=11-task-suspend-resume run` |
| [`12-software-timer`](12-software-timer/README.md) | Target | Dịch vụ bộ định thời phần mềm | `make EXAMPLE=12-software-timer run` |
| [`13-01-event-post`](13-01-event-post/README.md) | Target | Đăng sự kiện haievent từ ISR | `make EXAMPLE=13-01-event-post run` |
| [`13-02-active-object`](13-02-active-object/README.md) | Target | Active Object Ping–Pong | `make EXAMPLE=13-02-active-object run` |
| [`13-03-flat-state-machine`](13-03-flat-state-machine/README.md) | Target | Máy trạng thái phẳng | `make EXAMPLE=13-03-flat-state-machine run` |
| [`13-04-time-event`](13-04-time-event/README.md) | Target | Sự kiện thời gian haievent | `make EXAMPLE=13-04-time-event run` |
| [`13-05-publish-subscribe`](13-05-publish-subscribe/README.md) | Target | Publish–Subscribe và quyền sở hữu sự kiện động | `make EXAMPLE=13-05-publish-subscribe run` |
| [`13-06-event-driven-demo`](13-06-event-driven-demo/README.md) | Target | Demo haievent tích hợp | `make EXAMPLE=13-06-event-driven-demo run` |
| [`14-memory-allocator-lab`](14-memory-allocator-lab/README.md) | Host + Target | Bài thực hành bộ cấp phát bộ nhớ | `make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run` |
| [`15-kernel-benchmark`](15-kernel-benchmark/README.md) | Target | Benchmark kernel | `make EXAMPLE=15-kernel-benchmark run` |
| [`16-diagnostics-stress-stabilization`](16-diagnostics-stress-stabilization/README.md) | Host + Target | Chẩn đoán và ổn định bằng stress test | `make ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization run` |

## Cách đọc kết quả

- Một target **biên dịch PASS** chưa chứng minh runtime trên bo mạch PASS.
- Dòng `PASS` trong UART là checkpoint do chính example kiểm tra các invariant chính.
- Giá trị tick/counter có thể khác đôi chút; thứ tự sự kiện và quan hệ priority mới là tiêu chí quan trọng.
- Khi có `board_panic()`, giữ lại UART log, map file và fault record trước khi thay đổi code.
- Kiểm thử host với ASan/UBSan bổ sung kiểm tra bộ nhớ và bất biến, nhưng không thay thế kiểm thử timing trên Cortex-M3.

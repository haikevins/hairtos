# Nhật ký thay đổi

Tất cả những thay đổi đáng chú ý của `hairtos` đều được ghi lại trong tài liệu này.

Tài liệu được trình bày theo định dạng Keep a Changelog và project sử dụng Semantic Versioning.

## [Chưa phát hành]

### Thay đổi

- Bổ sung cơ chế `TARGET` và manifest `cmake/targets/` để port MCU mới mà không sửa Makefile hoặc CMakeLists.txt.
- Chuyển SysTick kernel adapter khỏi `kernel/` sang architecture port.
- Tổng quát hóa GPIO, UART và hardware timer API; clock ngoại vi và pin mapping được xử lý trong target implementation.
- Chuyển giới hạn stack, FPU và MPU capability sang `hr_port_config.h`.
- Chuyển metadata bộ nhớ và benchmark marker sang board abstraction.

- Viết lại các file README tại thư mục gốc, cấu hình VS Code, driver, kernel benchmark và memory allocator bằng tiếng Việt với bố cục đầy đủ, phù hợp với vai trò của từng thành phần; giữ nguyên nội dung tài liệu hiện có trong `examples/` và `docs/`.
- Viết lại toàn bộ `examples/*/README.md` bằng tiếng Việt theo một bố cục thống nhất gồm mười phần, trình bày mục tiêu, cấu hình, luồng thực thi, API, lệnh build, kết quả mong đợi, tiêu chí PASS, xử lý lỗi và phạm vi của từng example.
- Xây dựng lại `examples/README.md` thành tài liệu chỉ mục trung tâm, phân loại rõ các example chạy trên host và target.

## [1.0.0-rc1] - 2026-07-27

### Bổ sung

- Kernel Cortex-M3 theo định hướng static-first, hỗ trợ task, scheduler, timeout, queue, semaphore, mutex, software timer, diagnostics và lưu fault record qua reset.
- Framework sự kiện `haievent` hỗ trợ Active Object, flat state machine, time event và publish/subscribe.
- Bộ kiểm thử host với sanitizer, scheduler stress xác định, memory allocator lab và các example benchmark sử dụng DWT.
- File `VERSION` tại thư mục gốc và tài liệu nhật ký thay đổi này.

### Thay đổi

- Sử dụng CMake làm nguồn cấu hình duy nhất cho việc lựa chọn example, module và source.
- Rút gọn Makefile gốc thành lớp giao diện dòng lệnh thống nhất bao quanh CMake.
- Giới hạn đường dẫn include `kernel/internal` và `haievent/internal` chỉ dành cho kernel, framework, tests và hai example chủ đích truy cập thành phần nội bộ.
- Tổ chức lại driver thành public interface, common code và implementation riêng theo SoC, giúp bổ sung SoC thứ hai mà không trộn lẫn các implementation.

### Loại bỏ

- Thư mục benchmark placeholder trùng lặp tại `tests/benchmark/`.
- Các tài liệu lỗi thời liên quan đến repository hygiene và đóng gói release.

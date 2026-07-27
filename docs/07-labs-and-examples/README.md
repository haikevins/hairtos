# 07 — Bài thực hành và ví dụ

## 1. Mục tiêu

Nhóm này liên kết lộ trình example 01–16 với allocator lab và các mục tiêu kiểm chứng tương ứng.

## 2. Nội dung

- [example-index.md](example-index.md)
- [memory-allocator-lab.md](memory-allocator-lab.md)
- [`../../examples/README.md`](../../examples/README.md)

## 3. Bố cục chung

Mỗi example README có mười phần: mục tiêu, kiến thức, cấu hình, luồng, API/source, build/run, output, PASS/troubleshooting, giới hạn và liên hệ lộ trình.

## 4. Target selection

Target examples dùng:

```bash
make TARGET=<target> EXAMPLE=<name> build
```

Host-only và dual examples dùng cùng command interface; `ENVIRONMENT` chỉ cần chỉ rõ khi example hỗ trợ cả host và target.

## 5. Port validation

Khi thêm target, chạy example theo thứ tự tăng dần. Không bắt đầu bằng image tích hợp nếu bare-metal clock/UART/tick chưa PASS.

# 00 — Tổng quan

## 1. Mục tiêu

Nhóm tài liệu này thiết lập phạm vi của project, dependency rules, cấu hình, coding standard, repository layout và roadmap.

## 2. Nội dung

- [architecture.md](architecture.md)
- [dependency-rules.md](dependency-rules.md)
- [project-layout.md](project-layout.md)
- [configuration.md](configuration.md)
- [coding-standard.md](coding-standard.md)
- [roadmap.md](roadmap.md)

## 3. Thứ tự đọc

Đọc `architecture.md` và `dependency-rules.md` trước; sau đó xem `project-layout.md` và `configuration.md`. `coding-standard.md` áp dụng khi chỉnh source, còn `roadmap.md` mô tả hướng phát triển.

## 4. Liên hệ với portability

Target manifest không làm thay đổi dependency direction. Kernel/framework không được phụ thuộc ngược vào `arch/`, `soc/`, `boards/` hoặc driver implementation. CMake chỉ binding các layer tại build time.

## 5. Kiểm tra

Khi thêm target mới, xác nhận layout, target discovery và include boundary vẫn đúng; application không được nhận internal include ngoài các example có chủ đích.

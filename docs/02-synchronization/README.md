# 02 — Đồng bộ hóa và IPC

## 1. Mục tiêu

Nhóm này mô tả queue, semaphore, mutex, software timer và suspend/resume.

## 2. Nội dung

- [queue.md](queue.md)
- [semaphore.md](semaphore.md)
- [mutex.md](mutex.md)
- [software-timer.md](software-timer.md)
- [suspend-resume.md](suspend-resume.md)

## 3. Dependency

Các primitive phụ thuộc task, scheduler, wait list, timeout và critical-section contract; chúng không phụ thuộc trực tiếp board hoặc peripheral driver.

## 4. ISR boundary

API `*_from_isr` dựa trên port ISR detection và deferred context switch. Target mới phải xác nhận interrupt priority model trước khi coi runtime là hợp lệ.

## 5. Portability

Không cần sửa logic queue/semaphore/mutex/timer khi port MCU cùng execution model. Chỉ architecture critical section, tick và context switch backend thay đổi.

## 6. Kiểm thử

Host tests bao phủ trạng thái và edge cases; target examples kiểm chứng scheduling/ISR integration.

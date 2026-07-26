# Suspend và resume task

## 1. Mục tiêu

Cho phép task context tạm dừng task READY, RUNNING hoặc BLOCKED mà không xóa TCB.

## 2. Suspend READY

Task bị remove khỏi ready queue và chuyển `SUSPENDED`, lưu resume state là READY.

## 3. Self-suspend RUNNING

Current task chuyển SUSPENDED, bị remove khỏi ready queue và pend PendSV. Khi được resume, nó tiếp tục sau lời gọi suspend.

## 4. Suspend BLOCKED

Task giữ wait-list/timeout membership và lưu resume state BLOCKED. Event hoặc timeout vẫn có thể hoàn tất trong lúc suspended; khi đó task trở thành logic `SUSPENDED(READY)` nhưng chưa vào ready queue.

## 5. Resume

- Resume state READY: insert ready queue.
- Resume state BLOCKED: tiếp tục chờ object/timeout.
- Nếu operation đã hoàn tất khi suspended: insert ready queue và trả kết quả khi task chạy lại.

Resume task priority cao hơn có thể yêu cầu preemption.

## 6. Bảo vệ

Không suspend idle, task CREATED, task đã SUSPENDED hoặc gọi từ ISR. Resume task không suspended trả invalid state.

## 7. Giới hạn

Không có `resume_from_isr()` và không có suspend-all scheduler API.

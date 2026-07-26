# Time, SysTick và timeout

## 1. Mục tiêu

Cung cấp tick 32-bit, delay blocking, timeout cho IPC và xử lý wrap-around.

## 2. Tick source

Từ Phase 7, strong `SysTick_Handler` gọi `hr_kernel_tick_from_isr()`. Tick mặc định 1 kHz, nên một tick tương đương khoảng 1 ms với cấu hình chuẩn.

`hr_time_now()` trả tick hiện tại.

## 3. Blocking delay

`hr_task_delay(ticks)` loại current task khỏi ready queue, thêm timeout node và chuyển state sang BLOCKED. `hr_task_delay_until()` tính deadline từ release trước để giảm drift periodic task.

## 4. Timeout list

Timeout node được sắp theo absolute wake tick trong current hoặc overflow list. Khi `now < last_tick`, kernel nhận biết wrap và hoán đổi lists.

## 5. Expiration

SysTick lấy các node hết hạn, gọi cleanup nếu task đang chờ object, cập nhật wait result và đưa task về READY hoặc `SUSPENDED(READY)`.

## 6. Race delay một tick

Nếu timeout hết hạn trước khi PendSV của operation block chạy, selector kiểm tra state hiện tại và không giả định task vẫn BLOCKED. Đây là trường hợp được regression test.

## 7. Timeout semantics

- `HR_NO_WAIT`: operation không block.
- finite tick: block và có thể trả `HR_ERROR_TIMEOUT`.
- `HR_WAIT_FOREVER`: block không gắn timeout node.

## 8. Software timer

Software timer dùng timeout mechanism riêng trong timer subsystem nhưng cùng tick source. Callback được defer tới timer-service task.

## 9. Giới hạn

Tick là 32-bit; khoảng wrap phụ thuộc tick rate. Timeout list giả định tick handler được gọi đều, không bỏ qua cả một vòng 32-bit.

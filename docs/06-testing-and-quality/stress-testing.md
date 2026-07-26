# Stress testing

## 1. Mục tiêu

Tìm lỗi state/list hiếm bằng operation sequence dài và deterministic.

## 2. Host stress

```bash
make ENVIRONMENT=host EXAMPLE=hairtos run
```

Stress ready-set/scheduler qua hàng trăm nghìn insert/remove/rotate/validate operations. Seed/sequence cố định giúp tái tạo lỗi.

## 3. Target stress

Example `hairtos` chạy queue, semaphore, mutex, software timer, timeout, preemption và health monitor đồng thời.

## 4. Tiêu chí PASS

- Không panic/assert.
- Health check luôn valid.
- Stack guard hợp lệ.
- Counters tiếp tục tăng.
- Producer/consumer và timer không stall.
- UART checkpoint xuất hiện theo chu kỳ.

## 5. Long-duration plan

Chạy ít nhất 8 giờ trên board, lưu UART log và reset reason. Thêm tải interrupt ngoài nếu sản phẩm thật có ISR dày.

## 6. Fault scenarios

- queue full/empty lặp lại;
- timeout gần tick wrap;
- waiter suspended khi event hoàn tất;
- nested mutex inheritance;
- timer pending backlog;
- event pool exhaustion.

## 7. Giới hạn

Deterministic host stress không mô phỏng chính xác interrupt timing phần cứng.

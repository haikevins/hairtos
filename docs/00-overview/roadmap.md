# Roadmap

## Lịch sử v1

| Mốc | Nội dung | Trạng thái |
|---:|---|---|
| 0 | Specification | Hoàn thành |
| 1 | Bare-metal foundation | Hoàn thành |
| 2 | Intrusive structures | Hoàn thành |
| 3 | TCB + initial stack | Hoàn thành |
| 4 | First task startup | Hoàn thành |
| 5 | Cooperative context switch | Hoàn thành |
| 6 | Priority scheduler | Hoàn thành |
| 7 | Tick + delay | Hoàn thành |
| 8 | Preemption + round-robin | Hoàn thành |
| 9 | Queue + blocking | Hoàn thành |
| 10 | Semaphore + mutex | Hoàn thành |
| 11 | Suspend/resume | Hoàn thành |
| 12 | Software timer | Hoàn thành |
| 13 | haievent | Hoàn thành |
| 14 | Allocator lab | Hoàn thành |
| 15 | Benchmark | Hoàn thành |
| 16 | Diagnostics/stabilization | Hoàn thành |

Các số trên là **lộ trình học**, không xuất hiện trong tên API/function nội bộ.

## Trạng thái release

Current version:

```text
1.0.0-rc1
```

Mục tiêu trước stable v1:

- chạy hardware regression trên target tham chiếu;
- soak test dài;
- retained-fault reset validation;
- đóng known-issues quan trọng.

## Version 2

Version 2 không nối tiếp bằng "Phase 17". Nó là một release architecture roadmap riêng.

Xem [`../09-version2/roadmap.md`](../09-version2/roadmap.md).

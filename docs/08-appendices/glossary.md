# Glossary

| Thuật ngữ | Giải thích |
|---|---|
| AO | Active Object: task + queue + state machine |
| Base priority | Priority cấu hình ban đầu của task |
| Effective priority | Priority hiện hành sau inheritance |
| Direct handoff | Chuyển resource/data trực tiếp cho waiter trước khi wake |
| Hardware frame | R0–R3, R12, LR, PC, xPSR do Cortex-M tự stack |
| Intrusive list | Node nằm bên trong object thay vì cấp phát riêng |
| MSP | Main Stack Pointer, dùng bởi handler và startup |
| PSP | Process Stack Pointer, dùng bởi hairtos task |
| PendSV | Exception priority thấp dùng context switch |
| Run-to-completion | Handler xử lý xong một event trước event tiếp theo |
| SVC | Supervisor Call dùng khởi chạy first task |
| Time slicing | Chia CPU giữa task cùng priority theo quantum |
| TCB | Task Control Block |
| Wait list | Danh sách task chờ object theo priority/FIFO |
| Watermark | Ước lượng stack đã dùng từ fill pattern |

# Risk register Version 2

| Risk | Tác động | Cách kiểm soát |
|---|---|---|
| HSM làm API quá phức tạp | Cao | Core subset trước, history sau |
| BASEPRI misconfig IRQ | Cao | Compile/runtime validation + docs |
| Tickless race | Cao | Model tests + hardware wake tests |
| Second target kéo theo HAL scope | Trung bình | Chỉ driver API cần thiết |
| Trace làm tăng latency | Trung bình | Compile-time/static/small records |
| Shared AO executor làm scope nổ | Cao | Để stretch/2.1 |
| V2 breaking quá nhiều | Cao | Compatibility policy |
| FPU port lỗi lazy stacking | Cao | Chỉ advertise sau dedicated tests |
| Manifest schema over-engineer | Trung bình | Đợi target thứ hai lộ duplication |
| Docs lệch source | Trung bình | Release doc/API checks |

## Nguyên tắc giảm risk

Mỗi milestone phải giữ all previous tests green. Không phát triển HSM, tickless, second port và shared executor cùng lúc trên một nhánh integration lớn.

## Feature kill criteria

Một planned feature có thể bị defer khỏi 2.0 nếu:

- chưa có use-case rõ;
- không test deterministic được;
- kéo dependency ngược;
- làm static memory không dự đoán được;
- làm chậm các mục tiêu bắt buộc như second target/HSM/validation.

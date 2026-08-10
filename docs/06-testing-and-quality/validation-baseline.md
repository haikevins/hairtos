# Validation baseline của audit hiện tại

Tài liệu này ghi lại điều **đã kiểm tra trong môi trường phân tích source**, không thay cho certification/hardware report.

## Source baseline

Version:

```text
1.0.0-rc1
```

Target discovered:

```text
bluepill_f103c8
```

Examples:

- 1 host-only;
- 2 dual host+target;
- 21 target firmware selections.

## Target cross-build

Toàn bộ target examples hiện tại đã cross-build thành công bằng Clang/LLD trong môi trường audit.

| Example | text | data | bss |
|---|---:|---:|---:|
| 01 | 2760 | 4 | 12 |
| 03 | 3384 | 4 | 784 |
| 04 | 6052 | 8 | 2072 |
| 05 | 6360 | 4 | 3224 |
| 06 | 6752 | 4 | 4248 |
| 07 | 7952 | 4 | 3224 |
| 08 | 8172 | 4 | 4640 |
| 09 | 10360 | 4 | 3952 |
| 10-01 | 9004 | 4 | 3592 |
| 10-02 | 10916 | 4 | 5184 |
| 11 | 9456 | 4 | 5024 |
| 12 | 10804 | 8 | 3984 |
| 13-01 | 11708 | 4 | 4344 |
| 13-02 | 11660 | 4 | 6056 |
| 13-03 | 11540 | 4 | 4328 |
| 13-04 | 12944 | 4 | 4328 |
| 13-05 | 13112 | 20 | 7016 |
| 13-06 | 15352 | 4 | 8872 |
| 14 | 5480 | 4 | 2572 |
| 15 | 20236 | 4 | 13976 |
| 16 | 19384 | 4 | 7320 |

Các con số phụ thuộc toolchain/config và chỉ dùng làm baseline so sánh.

## Host tests

Test executable chứa **64 tests**. Khi ASan runtime được preload đúng trong môi trường audit, 64/64 PASS, bao gồm deterministic scheduler stress.

Lời gọi CTest mặc định tại môi trường audit gặp loader warning trước test body:

```text
ASan runtime does not come first in initial library list
```

Đây là environment/toolchain-loader issue trong phiên audit; không được ghi thành "64 test source failed". Đồng thời cũng không nên che nó: release automation phải làm sanitizer invocation portable hơn.

## Chưa thực hiện trong audit

- flash Blue Pill thật;
- UART runtime capture;
- logic analyzer marker;
- reset retention test;
- physical fault injection;
- nhiều giờ soak;
- second hardware target.

## Ý nghĩa

Baseline mạnh ở compile + generic C test, nhưng hardware confidence vẫn là phần cần hoàn tất trước stable release.

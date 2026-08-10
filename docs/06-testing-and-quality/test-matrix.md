# Test matrix

| Thay đổi | Host unit | Stress | Target build | Hardware |
|---|---|---|---|---|
| Intrusive list | Bắt buộc | Nên | Kernel images | Không luôn cần |
| Scheduler | Bắt buộc | Bắt buộc | 05–08,16 | Bắt buộc |
| Timeout | Bắt buộc | Bắt buộc | 07,09,16 | Nên |
| Queue | Bắt buộc | Nên | 09,16 | Nên |
| Semaphore | Bắt buộc | Nên | 10-01,16 | ISR hardware nên |
| Mutex | Bắt buộc | Nên | 10-02,16 | Nên |
| Timer | Bắt buộc | Nên | 12,13,16 | Nên |
| Context port | Stack test | — | 04–08 | Bắt buộc |
| Fault backend | Diagnostics tests | — | 16 | Bắt buộc |
| haievent event ownership | Bắt buộc | Nên | 13-05/06 | Nên |
| FSM/AO | Bắt buộc | Nên | 13-* | Nên |
| Target manifest | — | — | all examples | Bắt buộc target mới |
| Driver | — | — | 01+ | Bắt buộc |
| Benchmark | Stats unit | — | 15 | Logic analyzer/timing |

## Version 2 quality target

V2 nên thêm CI matrix:

```text
Host GCC + ASan/UBSan
Host Clang + ASan/UBSan
Target GCC cross-build
Target Clang cross-build
All target manifests
Property/randomized tests
Hardware smoke
Scheduled soak
```

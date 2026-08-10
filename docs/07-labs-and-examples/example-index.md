# Example index

| Example | Môi trường | Capability chính | Port validation |
|---|---|---|---|
| 01 baremetal | Target | startup/clock/UART/GPIO/tick | Board/SoC |
| 02 structures | Host | ready/wait ordering | Generic C |
| 03 stack | Target | TCB + initial frame | Port stack |
| 04 first task | Target | first-task startup | SVC/port |
| 05 coop switch | Target | PendSV context | Save/restore |
| 06 priority | Target | fixed priority/FIFO | Scheduler |
| 07 delay | Target | tick/timeout | Tick adapter |
| 08 preemption | Target | preemption/slicing | Tick+PendSV |
| 09 queue | Target | blocking IPC | Kernel |
| 10-01 semaphore ISR | Target | ISR wake | IRQ contract |
| 10-02 mutex | Target | inheritance | Scheduler+mutex |
| 11 suspend | Target | admin state overlay | Task model |
| 12 timer | Target | timer-service | Tick+service |
| 13-01 event ISR | Target | AO post ISR | Framework/IRQ |
| 13-02 AO | Target | task+queue+FSM | Framework |
| 13-03 FSM | Target | ENTRY/EXIT | Framework |
| 13-04 time event | Target | timer→AO | Framework/timer |
| 13-05 pubsub | Target | reference ownership | Framework |
| 13-06 integration | Target | complete event app | Framework |
| 14 allocator | Host+Target | pool/first-fit | Lab |
| 15 benchmark | Target | clock/marker/stats | Target backend |
| 16 diagnostics | Host+Target | stress/fault/health | Integration |

## Command

```bash
make TARGET=bluepill_f103c8 EXAMPLE=<name> build
make TARGET=bluepill_f103c8 EXAMPLE=<name> run
```

Dual:

```bash
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run
make TARGET=bluepill_f103c8 ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run
```

## Khi thêm target

Minimum useful sequence:

```text
01 -> 03 -> 04 -> 05 -> 07 -> 08 -> 10-01 -> 12 -> 13-06 -> 15 -> 16
```

Nếu fail ở bước nào, không bỏ qua lên integration image.

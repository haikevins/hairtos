# Kernel benchmark

## Generic statistics

`benchmarks/kernel` chỉ tính sample statistics. Clock backend do target cung cấp.

## Target example metrics

Example 15 đo các path như:

- first task startup;
- critical section;
- scheduler;
- queue;
- semaphore;
- mutex;
- timer commands;
- yield round-trip;
- wake/preemption;
- haievent dispatch;
- timer jitter.

## DWT target

Blue Pill target dùng Cortex-M3 DWT CYCCNT.

## Board marker

Marker do `board_benchmark_marker_*` cung cấp; target hiện dùng PB0 nhưng benchmark generic không biết pin này.

## Method

UART không nằm trong vùng timestamp. Sample được lưu tĩnh rồi report sau.

Statistics:

```text
min
p50
mean
p95
max
```

## Không phải WCET proof

64 samples không đủ để chứng minh hard WCET. Interrupt load, compiler flags, Flash wait states và target clock ảnh hưởng kết quả.

## V2

- trace/benchmark clock capability metadata;
- automated result capture;
- context-switch latency under interrupt load;
- critical-section latency;
- power/tickless wake latency.

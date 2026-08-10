# Kernel lifecycle

## State

```text
RESET --hr_kernel_init()--> INITIALIZED --hr_kernel_start()--> RUNNING
  \                                                             |
   +-------------------------- PANIC <---------------------------+
```

`PANIC` là terminal logical state; không có API reset kernel trong cùng boot.

## RESET

Global kernel state mới reset. Chưa có scheduler/idle task registry hợp lệ cho application API.

## `hr_kernel_init()`

Initialization thực hiện:

- initialize scheduler ready queues/bitmap;
- initialize all-task list;
- initialize timeout system;
- reset current task;
- reset task count/tick/switch state;
- create static idle task;
- start/register idle task;
- reset timer system nếu feature được link;
- chuyển state thành INITIALIZED.

Application task nên được create/start sau `hr_kernel_init()` và trước `hr_kernel_start()`.

## INITIALIZED

Task có thể được register vào scheduler nhưng chưa chạy. Timer object có thể được create trong giai đoạn này.

`hr_kernel_start()`:

1. kiểm tra lifecycle;
2. chọn task READY priority cao nhất;
3. set task đó RUNNING/current;
4. gọi `hr_port_start_first_task()`;
5. trên target thành công, control không quay lại caller.

Nếu port start quay lại không hợp lệ, đó là lỗi.

## RUNNING

Các API scheduling/blocking mới có ý nghĩa runtime. Tick ISR update timeouts và scheduling decisions.

## Idle task

Idle task là fallback luôn tồn tại. Nó dùng priority `HR_CFG_IDLE_PRIORITY` và static stack riêng. Port target có thể dùng wait-for-interrupt trong idle loop.

Không được suspend idle task.

## PANIC

Diagnostics hoặc invariant failure có thể record panic. Fault handlers ghi retained record rồi halt theo port/board policy.

## Invariants lifecycle

- task count không vượt `HR_CFG_MAX_TASKS`;
- current task NULL trước RUNNING;
- RUNNING phải có đúng một current task;
- idle task phải valid;
- kernel start không được gọi hai lần.

## Source

```text
kernel/src/hr_kernel.c
kernel/internal/hr_kernel_internal.h
kernel/src/hr_task.c
arch/<port>/
```

## Test

Host tests kiểm tra lifecycle và selector policy. Target example 04 kiểm tra first-task startup thật.

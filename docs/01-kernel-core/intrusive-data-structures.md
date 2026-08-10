# Intrusive data structures

## Vì sao intrusive?

RTOS cần move task giữa ready/wait/timeout lists nhưng không muốn cấp phát list node động. Node nằm trực tiếp trong TCB.

## List

`hr_list_t` là circular doubly-linked list có sentinel root và size. `hr_list_node_t` biết:

```text
previous
next
list
owner
```

`owner` đưa scheduler từ node về TCB/object.

## Membership rule

Một node chỉ được nằm trong một list tại một thời điểm. Vì task cần nhiều membership đồng thời, TCB có node riêng cho ready, wait, timeout và all-task.

## Ready set

Một FIFO list cho mỗi priority + bitmap:

```text
P0 -> ...
P1 -> A -> B
P2 -> ...
...
```

Bitmap chỉ ra priority có member. Highest selection chọn priority số nhỏ nhất.

## Wait list

Waiters được insert theo effective priority. Cùng priority giữ FIFO.

Điều này dùng chung cho queue/semaphore/mutex.

## Timeout lists

Timeout system có current list và overflow list. Node được ordered theo absolute wake tick trong epoch tương ứng.

## Validate

List/scheduler/wait/timeout có validate functions để check:

- backlink symmetry;
- owner/list pointer;
- size;
- bitmap/list consistency;
- ordering.

## Failure modes cần tránh

- double insert;
- remove node không linked;
- move node nhưng quên update bitmap;
- effective priority đổi nhưng wait node không reinsert;
- timeout completion nhưng wait node còn linked.

## Source

```text
kernel/src/hr_list.c
kernel/src/hr_scheduler.c
kernel/src/hr_wait.c
kernel/src/hr_timeout.c
```

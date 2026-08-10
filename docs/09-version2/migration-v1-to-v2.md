# Migration từ v1 sang v2

Tài liệu này mô tả mục tiêu migration dự kiến; exact API chỉ được chốt khi v2 implementation tồn tại.

## Application dùng task/IPC thuần

Mục tiêu: ít hoặc không đổi source.

Kiểm tra:

- interrupt priority config;
- task-return behavior;
- config macro mới;
- target manifest v2.

## Application dùng flat FSM

Mục tiêu: flat handler tiếp tục chạy như HSM depth=1.

Có thể cần đổi state declaration nếu HSM metadata dùng explicit state object. Compatibility macro/helper nên giảm migration.

## Application dùng Active Object

Dedicated-task AO vẫn support. State handler blocking misuse có thể bị debug assert ở v2; code đang delay/wait trong handler phải refactor sang Time Event hoặc event-driven continuation.

## Event ownership

Không thay core retain/release semantics. Deferred events sẽ thêm ownership state mới; application chỉ bị ảnh hưởng nếu dùng defer API.

## Target port

Target manifest có thể cần chuyển sang schema mới/fragments. Kernel source không đổi.

Port critical section có thể chuyển PRIMASK→BASEPRI; IRQ priorities gọi kernel API phải cấu hình đúng.

## Diagnostics

Panic record version tăng nếu thêm build ID/trace pointers. Boot reader phải check version trước parse.

## Recommended migration workflow

```text
1. build v1 tests clean
2. switch v2 branch
3. migrate config/target manifest
4. compile task/IPC apps
5. fix AO blocking violations
6. migrate FSM declarations if needed
7. run host tests
8. target smoke
9. soak/fault
```

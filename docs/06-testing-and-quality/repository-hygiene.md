# Repository hygiene

## 1. Mục tiêu

Ngăn placeholder, ghost API, build artifact và source không được sử dụng quay lại release.

## 2. Kiểm tra

```bash
make repository-hygiene-check
```

Script quét placeholder text, obsolete paths, forbidden artifact và source coverage.

## 3. Quy tắc file mới

Mỗi source phải thuộc ít nhất một flow:

- target Makefile/CMake;
- host tests;
- stress test;
- validation/packaging tool;
- lab hoặc benchmark được document rõ.

## 4. Release package

```bash
make package-release
```

Loại `.git`, `build`, `out`, `dist`, cache Python và OS metadata.

## 5. Ghost API

Không public include/typedef cho feature chưa có implementation. Nếu feature là roadmap tương lai, ghi trong docs/issues thay vì tạo header rỗng.

## 6. Documentation

Tài liệu hiện hành nằm theo subsystem; phase history nằm riêng. Link nội bộ phải hợp lệ và không nhắc path đã xóa.

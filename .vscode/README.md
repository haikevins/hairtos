# IntelliSense cho VS Code

Sử dụng cấu hình **hairtos - All Sources** khi duyệt toàn bộ repository.
Cấu hình này chỉ cung cấp các thư mục include nội bộ cho trình soạn thảo, nhờ đó
các file nằm ngoài example CMake đang được chọn không xuất hiện lỗi include giả.
Ranh giới biên dịch thực tế vẫn được CMake bảo vệ bằng include directory của từng target.

Để tạo compile definition chính xác cho từng file, chạy:

```bash
make EXAMPLE=16-diagnostics-stress-stabilization ENVIRONMENT=target TOOLCHAIN=clang intellisense
```

Sau đó chọn cấu hình **hairtos - Active CMake Build**.

Sau khi thay cấu hình, mở Command Palette của VS Code và chạy lần lượt:

1. `C/C++: Select a Configuration`
2. Chọn `hairtos - All Sources`
3. `C/C++: Reset IntelliSense Database`
4. `Developer: Reload Window`

Hãy mở thư mục gốc của repository, nơi chứa `Makefile`; không mở riêng một thư mục example.
Khi sử dụng IntelliSense của Microsoft C/C++, hãy tắt extension clangd trong workspace này.

# ProcessDumper

Một công cụ giúp dump các process bị mã hóa từ bộ nhớ. Nó theo dõi các trang bộ nhớ trong thời gian thực, đợi chúng được giải mã và xây dựng lại thành một file PE hoàn chỉnh. Hoạt động tốt với các ứng dụng được bảo vệ bởi Hyperion và Theia.

## Cách thức hoạt động

1. Một driver hạt nhân (kernel driver) đọc bộ nhớ từ process mục tiêu.
2. Bộ theo dõi trang (page monitor) giám sát các trang ở trạng thái `NOACCESS` khi chúng trở nên có thể đọc được (= đã được giải mã).
3. Sau khi hoàn tất quá trình chờ, công cụ sẽ xây dựng lại file PE với các header, import và section đã được sửa lỗi.
4. Bạn sẽ nhận được một bản dump sạch dưới dạng file `.exe`.

## Tính năng

- **Truy cập bộ nhớ qua Kernel driver** — sử dụng driver IOCTL (có thể nạp bằng KDmapper), giúp hoạt động ngay cả khi các handle ở usermode bị anti-cheat chặn.
- **Theo dõi giải mã trang** — bắt các trang khi chúng chuyển từ trạng thái mã hóa sang có thể đọc được.
- **Xử lý Import** — quét vùng `.rdata` để tìm địa chỉ các hàm exported, xây dựng section `.rimport` mới với IAT chuẩn và vá lại tất cả các tham chiếu `call`/`jmp`.
- **Dọn dẹp thư mục ngoại lệ (Exception directory)** — loại bỏ các mục `RUNTIME_FUNCTION` bị lỗi khỏi bản dump.
- **Ngưỡng tự động dừng (Auto-stop threshold)** — thiết lập mục tiêu như 50% và công cụ sẽ tự động dừng khi đủ số lượng trang đã được giải mã.
- **Dự phòng file trên đĩa (Disk file fallback)** — nếu không thể đọc một trang từ bộ nhớ, công cụ sẽ cố gắng lấy nó từ file PE gốc trên đĩa để thay thế.

## Cách sử dụng

```
ProcessDumper.exe <process.exe> [-t threshold]
```

- `-t` — tự động dừng ở một tỷ lệ phần trăm giải mã nhất định. `0.5` = 50%, `1.0` = 100%. Nếu không dùng tham số này, hãy nhấn **F7** để dừng thủ công.

**Ví dụ:**
```
ProcessDumper.exe game.exe -t 0.5
```

## Biên dịch (Building)

- Mở `ProcessDumper.sln` trong Visual Studio.
- Build cả hai project: `ProcessDumper` (usermode) và `IOCTL Driver` (kernel).
- Map driver bằng KDmapper trước khi chạy trình dumper.

## Ghi danh (Credits)

- Các tính năng xử lý import, sửa thư mục ngoại lệ, ngưỡng giải mã và dự phòng file trên đĩa được truyền cảm hứng từ Vulkan của atrexus.

---
Bản ReadMe này được tạo bởi Claude Opus 4.6 (vì tác giả lười :) )

# Hướng dẫn Kiểm tra và Sử dụng Usermode (hyper-reV)

Tài liệu này hướng dẫn cách kiểm tra sự ổn định của Hypervisor và sử dụng các tập lệnh cơ bản trong môi trường Usermode.

## 1. Chuẩn bị
*   Đảm bảo bạn đã khởi động máy với Logo **GZ-Cheat** hiện lên.
*   Chạy **`bin\usermode.exe`** với quyền **Administrator**.

## 2. Các bước kiểm tra ổn định (Health Check)

### Bước 1: Kiểm tra kết nối
Khi bật usermode, nếu bạn thấy dòng chữ `Ready!` và dấu nhắc `>`, nghĩa là Hypervisor đang hoạt động.
*   **Lệnh:** `hfpc`
*   **Mục đích:** Kiểm tra số lượng trang nhớ trống của Hypervisor. Nếu con số này lớn và ổn định, hệ thống đang rất khỏe.

### Bước 2: Kiểm tra khả năng đọc Kernel
*   **Lệnh:** `lkm`
*   **Mục đích:** Liệt kê các Driver đang chạy. Nếu bạn thấy danh sách hiện ra (ntoskrnl.exe, win32k.sys...), nghĩa là khả năng đọc bộ nhớ ảo (Virtual Memory) đang hoàn hảo.

## 3. Danh mục các lệnh thường dùng

| Lệnh | Mô tả | Ví dụ sử dụng |
| :--- | :--- | :--- |
| `lkm` | Liệt kê driver hệ thống | `lkm` |
| `kme` | Xem các hàm của driver | `kme ntoskrnl.exe` |
| `fl` | Xem nhật ký hoạt động | `fl` |
| `gva` | Tìm địa chỉ của một hàm | `gva ntoskrnl.exe!PsGetCurrentProcess` |
| `dkm` | Dump driver ra ổ cứng | `dkm ntoskrnl.exe C:\Dumps\` |

## 4. Các lệnh can thiệp sâu (Dành cho nhà phát triển)
*   **`rgvm` / `wgvm`**: Đọc/Ghi bộ nhớ ảo của bất kỳ tiến trình nào (cần có CR3).
*   **`akh`**: Đặt Hook tàng hình vào Kernel. Đây là lệnh mạnh nhất để bắt đầu làm cheat.
*   **`hgpp`**: Ẩn một trang nhớ vật lý khỏi tầm mắt của Anti-cheat.

---
**Lưu ý quan trọng:** Mọi hành động can thiệp bộ nhớ Kernel (`wgvm`, `akh`) đều có rủi ro gây xanh màn hình (BSOD) nếu địa chỉ nhập vào bị sai. Hãy luôn kiểm tra địa chỉ bằng `gva` trước khi thực hiện.

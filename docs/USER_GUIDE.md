# hyper-reV: Hướng Dẫn Cài Đặt và Sử Dụng (Dành Cho Người Dùng)

Tài liệu này hướng dẫn bạn cách cài đặt hyper-reV vào hệ thống để bắt đầu quá trình Introspection và Reverse Engineering.

---

## 1. Chuẩn Bị File (Cấu Trúc Thư Mục)
Đảm bảo thư mục **`bin`** của bạn chứa đầy đủ 4 tệp tin quan trọng sau:
*   `uefi-boot.efi`: Driver khởi động hệ thống.
*   `hyperv-attachment.dll`: Module cốt lõi của Hypervisor.
*   `loader.exe`: Trình cài đặt tự động (Yêu cầu quyền Admin).
*   `usermode.exe`: Ứng dụng điều khiển và xem Log sau khi cài đặt.

---

## 2. Thiết Lập Hệ Thống (Bắt Buộc)

### Đối Với Máy Thật (Bare-metal):
*   **BIOS/UEFI Setup**: 
    *   Tắt **Secure Boot** (Khuyên dùng để tránh các lỗi chữ ký số).
    *   Bật **Intel VT-x** hoặc **AMD-V** (Virtualization Technology).
*   **Windows**:
    *   Bật tính năng **Hyper-V** trong *Windows Features*.

### Đối Với Máy Ảo (VMware/Hyper-V VM):
*   **Settings -> Processors**: Tích chọn **"Virtualize Intel VT-x/EPT or AMD-V/RVI"**.
*   **Settings -> Options -> Advanced**: Chọn **Firmware: UEFI** (Tắt Secure Boot).

---

## 3. Quy Trình Cài Đặt (3 Bước)

### Bước 1: Copy Thư Mục
Copy toàn bộ thư mục **`bin`** vào máy tính mục tiêu (ví dụ: `C:\hyper-reV\`).

### Bước 2: Chạy Loader
Chuột phải vào tệp **`loader.exe`** và chọn **Run as Administrator**.
*   Nếu thành công, bạn sẽ thấy thông báo: `[+++] DEPLOYMENT SUCCESSFUL [+++]`.
*   Loader sẽ tự động sao lưu bootloader gốc của Windows và thay thế bằng bản của dự án.

### Bước 3: Khởi Động Lại (Activate)
Chọn **Restart** máy tính. Sau khi restart, Windows sẽ khởi động qua lớp Hypervisor của hyper-reV.

---

## 4. Kiểm Tra và Debug

### Sử Dụng Usermode App:
1.  Mở **`usermode.exe`** với quyền Administrator.
2.  Gõ lệnh **`fl`** (Flush logs) để xem các bản tin từ Hypervisor.
3.  Để đặt hook thử nghiệm: `akh ntoskrnl.exe!PsLookupProcessByProcessId --monitor`

### Debug Qua WinDbg (Nâng cao):
Nếu bạn đã thiết lập Serial Port (COM) trong máy ảo:
1.  Kết nối WinDbg (Host) với Named Pipe của VM (vd: `\\.\pipe\windbg`).
2.  Bạn sẽ thấy các log hệ thống hiện lên ngay từ giai đoạn khởi động.

---

## 5. Xử Lý Lỗi (Troubleshooting)

*   **Lỗi "Access is denied" khi chạy Loader**: Hãy chắc chắn bạn đã Run as Administrator.
*   **Máy không vào được Windows sau khi Restart**: 
    1.  Vào BIOS và tắt Secure Boot. 
    2.  Nếu vẫn lỗi, dùng USB cứu hộ để đổi tên `Z:\EFI\Microsoft\Boot\bootmgfw.original.efi` về lại `bootmgfw.efi` để khôi phục Windows.
*   **Usermode báo lỗi "Hypervisor not found"**: Kiểm tra xem Hyper-V đã được bật trong Windows chưa.

---
*Chúc bạn có trải nghiệm Reverse Engineering thú vị với hyper-reV!*

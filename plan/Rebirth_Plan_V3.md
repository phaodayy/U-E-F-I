# 📝 Kế hoạch Tái sinh PUBG-2 Hypervisor (V3.0 - Stealth Edition)

Dự án này tập trung vào việc loại bỏ các dấu hiệu nhận diện đặc trưng (Signatures) và thay đổi phương thức giao tiếp lộ liễu để vượt qua các lớp bảo vệ mới nhất của BattlEye và EAC.

---

## 🛠️ Phân nhiệm vụ và Tiến độ

### 1. 👻 Giao tiếp ẩn danh (Stealth Hypercall)
*   [x] **Loại bỏ CPUID/VMCALL:** Chuyển sang dùng cơ chế `CALL` Gateway.
*   [x] **EPT Hook Gateway:** Đã triển khai bẫy `INT3` trên Shadow Page của `NtUserGetForegroundWindow`.
*   [x] **Manual EAT Parser:** Tìm địa chỉ hàm mà không dùng Windows API.
*   [x] **Tự động Fallback:** Đã có cơ chế tự động chuyển từ CPUID (Handshake) sang Stealth Mode.

### 2. 🔐 Mã hóa & Bảo mật Dữ liệu
*   [ ] **String Encryption:** Mã hóa toàn bộ chuỗi hệ thống (TslGame, Driver names, v.v.) bằng `skCrypt`.
*   [ ] **Offset Obfuscation:** Mã hóa các giá trị offset tĩnh trong `pubg_config.hpp` (XOR động khi khởi chạy).
*   [ ] **SDK Strip:** Loại bỏ các thông tin debug, PDB rác trong quá trình build.

### 3. 🛡️ Bảo mật Tiến trình (Process Integrity)
*   [ ] **Remove ntdll Patching:** Ngừng việc patch trực tiếp vào `ntdll.dll` để tránh bị AC quét Integrity.
*   [ ] **Anti-Debug Refactor:** Sử dụng Hypervisor để xử lý các bẫy Debug thay vì dùng code Usermode.
*   [ ] **Process Identity:** Ngụy trang `GameOverlay.exe` thành các tiến trình hệ thống vô hại.

### 4. ⚡ Tối ưu hóa Hiệu năng (Timing Evasion)
*   [ ] **Bulk Reading:** Xây dựng cấu trúc dữ liệu lớn để đọc toàn bộ Actor trong 1 lần gọi Hypercall.
*   [ ] **Cache System:** Giảm tần suất truy cập memory bằng cách cache CR3 và BaseAddress.
*   [ ] **Latency Jitter:** Thêm độ trễ ngẫu nhiên siêu nhỏ giữa các lần quét để làm giả hành vi của người dùng thật.

---

## 📅 Roadmap thực hiện

| Giai đoạn | Nội dung | Trạng thái |
| :--- | :--- | :--- |
| **P1: Communication** | Thay thế CPUID bằng EPT Hook | 🔄 Đang triển khai |
| **P2: Obfuscation** | Mã hóa chuỗi và Offsets | ⏳ Chờ |
| **P3: Integration** | Cập nhật GameOverlay và Loader | ⏳ Chờ |
| **P4: Final Test** | Kiểm tra độ trễ và độ an toàn trên game | ⏳ Chờ |

---

> [!IMPORTANT]
> Tuyệt đối không để chuỗi "PUBG" hoặc các pattern gợi nhớ xuất hiện trong tên file, tiêu đề cửa sổ, hoặc metadata của project sau khi tái sinh.

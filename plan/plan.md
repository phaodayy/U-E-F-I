# UnifiedTelemetryNode: Anti-Ban & Stealth Overhaul Plan

Tài liệu này vạch ra lộ trình giải cứu dự án `UnifiedTelemetryNode` khỏi tình trạng bị ban vĩnh viễn, bằng cách loại bỏ các kỹ thuật cũ dễ bị phát hiện và tích hợp hoàn toàn cơ chế ẩn danh (Stealth) cấp cao từ mô hình `Game-2` (PubgExt).

## Mục Tiêu Xóa Bỏ Điểm Yếu Chết Người
*   **Gỡ bỏ API thao túng cửa sổ:** Không sử dụng `SetWindowLong` để bơm cờ `WS_EX_TRANSPARENT`.
*   **Gỡ bỏ API DWM dễ nhận diện:** Ngừng sử dụng `DwmExtendFrameIntoClientArea`.
*   **Gỡ bỏ chuỗi tĩnh (Static Strings) thô:** Không để văn bản/text lộ liễu trong Memory.

---

## Các Giai Đoạn Triển Khai

### Giai Đoạn 1: Discord Overlay Hijack (Hoàn thành)
Đây là cốt lõi để Overlay trở nên "Vô hình". Kỹ thuật này không tạo hay sửa đổi thuộc tính của bất kỳ cửa sổ nào, mà vẽ trực tiếp thông qua luồng hợp pháp của ứng dụng Discord.
*   **[x] Bước 1.1:** Dọn dẹp hàm `Initialize` và `RenderFrame` trong `overlay_menu.cpp`. Loạt bỏ các đoạn mã tìm và sửa window (`FindOverlayForGame`, `SetClickable`).
*   **[x] Bước 1.2:** Port file `DiscordOverlay.h` từ `Game-2` sang `telemetry/overlay`.
*   **[x] Bước 1.3:** Tích hợp logic tìm kiếm `Chrome_WidgetWin_1` (Cửa sổ của Discord Overlay).
*   **[x] Bước 1.4:** Thiết lập SwapChain và Render Target trỏ trực tiếp vào Discord Overlay thay vì tạo lớp D3D11 đè lên game. Cấu trúc lại vòng lặp `Render` trong `main.cpp` để chạy qua Callback của Discord Hijacker.

### Giai Đoạn 2: Memory Resiliency & Tự phục hồi CR3 (Hoàn thành)
Việc dữ liệu sai lệch khi hệ thống swap bảng trang (Page Table) gây nhiễu loạn hoặc crash, tạo vết nứt để Anti-cheat phát hiện sự thâm nhập bộ nhớ.
*   **[x] Bước 2.1:** Cập nhật file `hyper_process.cpp` bổ sung cơ chế kiểm đếm lỗi dịch địa chỉ (Translation Faults).
*   **[x] Bước 2.2:** Xây dựng logic tự động dò tìm lại `DirectoryTableBase` (FixCR3) bất cứ khi nào số lỗi vượt qua ngưỡng cho phép (Ví dụ: >10 lần trượt). Khôi phục đường truyền dữ liệu mà không làm sập tiến trình.

### Giai Đoạn 3: Metadata / String Encryption (XorStr) (Hoàn thành)
Chống lại các engine dò quét chữ ký bộ nhớ (Memory Signature Scanner) và quét chuỗi tĩnh (Static String Scanner).
*   **[x] Bước 3.1:** Đảm bảo `skCrypt` (hoặc `XorStr.h` từ Game-2) được áp dụng làm chuẩn mã hóa chuỗi tại thời điểm biên dịch (Compile-time) cho toàn dự án.
*   **[x] Bước 3.2:** Mở rộng phạm vi mã hóa (Càn quét lõi Core SDK). Không chỉ dừng ở lớp bảo vệ hay giao diện (Menu), cần bọc XorStr toàn bộ các chuỗi giao tiếp sâu bên trong cấu trúc hệ thống:
    *   **[x] Logic tương tác game:** Tên mục tiêu (`TslGame.exe`), các lớp vật thể (`Bone_Head`, `Bone_Pelvis`, `UWorld`, `GNames`).
    *   **[x] Cơ chế Hypervisor:** Các chuỗi định danh Driver, SymLink, tên Pipe/Mutex (VD: `\\.\Dumper`, `\\.\KsDumper`).
    *   **[x] Debug/Console Logs:** Mọi văn bản xuất ra trong quá trình chạy bộ (Logger Output).
*   **[x] Bước 3.3:** Kiểm tra chéo (Cross-check) và rà soát file `*.cpp`, `*.hpp` (đặc biệt trong thư mục `sdk/`) để quét sạch các chuỗi "PlainText" chưa được bọc `skCrypt`.
*   **[x] Mục tiêu:** Nhổ tận gốc "dấu vân tay tĩnh" (Static Signature). Kẻ thù khi dump memory của Node sẽ bị vô hiệu hóa hoàn toàn khả năng đọc mã (chỉ thấy các chuỗi rác vô nghĩa).

### Giai Đoạn 4: Humanized Signaling (Chống phân tích hành vi) (Đang chờ)
Giảm rủi ro bị AI khoanh vùng do thao tác giả lập mượt hoặc tuyến tính một cách thiếu tự nhiên.
*   **[ ] Bước 4.1:** Sử dụng `mouse_event` dưới ranh giới Driver thay vì User-mode API, kèm theo các hàm phá vỡ tính đồng nhất của các tham số tọa độ (Áp dụng thuật toán Randomize Mouse Vector khi truyền tọa độ telemetry).

---

*Trạng thái: Đã hoàn thành Giai đoạn 1, 2, 3 rực rỡ. Hệ thống đạt trạng thái "Zero-String" và tàng hình trên Discord Overlay. Sẵn sàng cho Giai đoạn 4.*

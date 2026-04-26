# UnifiedTelemetryNode: Anti-Ban & Stealth Overhaul Plan

Tài liệu này vạch ra lộ trình giải cứu dự án `UnifiedTelemetryNode` khỏi tình trạng bị ban vĩnh viễn, bằng cách loại bỏ các kỹ thuật cũ dễ bị phát hiện và tích hợp hoàn toàn cơ chế ẩn danh (Stealth) cấp cao từ mô hình `Game-2` (PubgExt).

## Mục Tiêu Xóa Bỏ Điểm Yếu Chết Người
*   **Gỡ bỏ API thao túng cửa sổ:** Không sử dụng `SetWindowLong` để bơm cờ `WS_EX_TRANSPARENT`.
*   **Gỡ bỏ API DWM dễ nhận diện:** Ngừng sử dụng `DwmExtendFrameIntoClientArea`.
*   **Gỡ bỏ chuỗi tĩnh (Static Strings) thô:** Không để văn bản/text lộ liễu trong Memory.

---

## Các Giai Đoạn Triển Khai

### Giai Đoạn 1: Discord Overlay Hijack (Quan trọng nhất)
Đây là cốt lõi để Overlay trở nên "Vô hình". Kỹ thuật này không tạo hay sửa đổi thuộc tính của bất kỳ cửa sổ nào, mà vẽ trực tiếp thông qua luồng hợp pháp của ứng dụng Discord.
*   **Bước 1.1:** Dọn dẹp hàm `Initialize` và `RenderFrame` trong `overlay_menu.cpp`. Loạt bỏ các đoạn mã tìm và sửa window (`FindOverlayForGame`, `SetClickable`).
*   **Bước 1.2:** Port file `DiscordOverlay.h` từ `Game-2` sang `telemetry/overlay`.
*   **Bước 1.3:** Tích hợp logic tìm kiếm `Chrome_WidgetWin_1` (Cửa sổ của Discord Overlay).
*   **Bước 1.4:** Thiết lập SwapChain và Render Target trỏ trực tiếp vào Discord Overlay thay vì tạo lớp D3D11 đè lên game. Cấu trúc lại vòng lặp `Render` trong `main.cpp` để chạy qua Callback của Discord Hijacker.

### Giai Đoạn 2: Memory Resiliency & Tự phục hồi CR3
Việc dữ liệu sai lệch khi hệ thống swap bảng trang (Page Table) gây nhiễu loạn hoặc crash, tạo vết nứt để Anti-cheat phát hiện sự thâm nhập bộ nhớ.
*   **Bước 2.1:** Cập nhật file `hyper_process.cpp` bổ sung cơ chế kiểm đếm lỗi dịch địa chỉ (Translation Faults).
*   **Bước 2.2:** Xây dựng logic tự động dò tìm lại `DirectoryTableBase` (FixCR3) bất cứ khi nào số lỗi vượt qua ngưỡng cho phép (Ví dụ: >10 lần trượt). Khôi phục đường truyền dữ liệu mà không làm sập tiến trình.

### Giai Đoạn 3: Metadata / String Encryption (XorStr)
Chống lại các engine dò quét chữ ký bộ nhớ (Memory Signature Scanner).
*   **Bước 3.1:** Đưa header `XorStr` (bào chế từ Game-2) hoặc bộ mã hóa tương đương vào dự án.
*   **Bước 3.2:** Bọc lại toàn bộ hệ thống tên hiển thị, tên module (như `TslGame.exe`), và các thông báo debug bằng Macro Xor để mã hóa tại thời điểm biên dịch (Compile-time encryption).

### Giai Đoạn 4: Humanized Signaling (Chống phân tích hành vi)
Giảm rủi ro bị AI khoanh vùng do thao tác giả lập mượt hoặc tuyến tính một cách thiếu tự nhiên.
*   **Bước 4.1:** Sử dụng `mouse_event` dưới ranh giới Driver thay vì User-mode API, kèm theo các hàm phá vỡ tính đồng nhất của các tham số tọa độ (Áp dụng thuật toán Randomize Mouse Vector khi truyền tọa độ telemetry).

---

*Trạng thái: Đã thống nhất kế hoạch. Chuẩn bị thực thi Giai đoạn 1.*

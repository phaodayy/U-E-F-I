# PUBG-2 Project Overview

Hệ thống **PUBG-2** là một giải pháp hỗ trợ (cheat/tool) cao cấp cho game PUBG.

## ⚠️ Quy tắc làm việc (Rules)
- **KHÔNG ĐƯỢC PHÉP COMMIT** (git commit) trừ khi có lệnh trực tiếp từ USER.

## 🚀 Kiến trúc & Công nghệ
- **DMA Core**: Sử dụng `DMALibrary` giao tiếp với phần cứng (Screamer/Raptor DMA) qua `vmmwin.dll`.
- **Game Engine**: Dựa trên Unreal Engine 4. Hỗ trợ đầy đủ các thành phần `UWorld`, `GNames`, `GObjects`.
- **Security**:
  - **Xenuine Decryption**: Cơ chế giải mã tùy chỉnh cho các pointer nhạy cảm.
  - **Spoof Call**: Sử dụng gadget call để ẩn stack trace.
  - **Memory Encryption**: Giải mã `CIndex` và dữ liệu `Health` bằng các thuật toán XOR/Rotation riêng biệt.
- **Input System**:
  - Tích hợp **KmBoxNet** (Hardware Mouse/Keyboard) cho việc aimbot cực kỳ an toàn.
  - **Kernel Mouse Injection**: Driver tùy chỉnh hỗ trợ di chuyển chuột mức kernel (ServiceCallback) để vượt qua các rào cản phần mềm.
  - Hỗ trợ Driver **Logitech** như một phương án dự phòng.

## 🛠 Các thành phần chính của SDK
- `pubg_config.hpp`: Chứa version (`2603.x`) và toàn bộ offset thô.
- `Players.h` & `Actors.h`: Logic lọc và xử lý thực thể trong game.
- `AimBot.h`: Thuật toán bám mục tiêu, tính toán dự đoán (prediction) và smoothing.
- `MeshPatcher.h`: Can thiệp vào mesh để xử lý kiểm tra tầm nhìn (Visible Check).
- `WebRadar.h`: Truyền dữ liệu ra Web để xem radar trên thiết bị khác (điện thoại/máy tính bảng).

---

# 🎯 Các kỹ năng hỗ trợ (Skills)

Tôi có khả năng hỗ trợ bạn thực hiện các tác vụ sau một cách tự động và chính xác:

### 1. Quản lý Offsets & Giải mã
- **Cập nhật Offsets**: Tự động cập nhật `pubg_config.hpp` khi có dữ liệu mới từ tool dump.
- **Phân tích Decryption**: Cập nhật hàm `decrypt_cindex` và các `HealthKey` (0-15) khi game thay đổi logic mã hóa.
- **Sửa lỗi GNames**: Xử lý các thay đổi trong cấu trúc `FName` hoặc `GNames` pool.

### 2. Tinh chỉnh Aimbot & Movement
- **Smoothing & Prediction**: Điều chỉnh tham số trong `AimBot.h` để tránh bị phát hiện bởi phân tích hành vi (heuristic).
- **Recoil Control**: Tinh chỉnh `autoRecoil.h` và `RecoilValueVector`.
- **Visibility Check**: Sửa lỗi tính toán `LastRenderTime` hoặc `Mesh` visibility.

### 3. Phát triển Overlay & Radar
- **ImGui Customization**: Thêm menu, thay đổi font, màu sắc và kiểu vẽ (Skeleton, Box, Health bar).
- **Web Radar Expansion**: Thêm các loại item mới (Flare gun, 8x scope...) vào hệ thống radar.

### 4. Xây dựng & Triển khai (Build & Deploy)
- **Visual Studio Build**: Thực hiện build solution `phao_final.sln` qua MSBuild.
- **Dependency Check**: Kiểm tra và đảm bảo các file `vmm.dll`, `dbghost.exe`, `leechcore.dll` có mặt đúng vị trí.

### 6. Phát triển Macro (Mới)
- **Chuẩn hóa Gun Data**: Đã chuyển toàn bộ database súng từ tiếng Trung sang tiếng Anh (`dataMacro/GunData/`).
- **Driver Mouse Pull**: Triển khai `HandleMouseMove` trực tiếp trong kernel driver, tích hợp vào `MouseDispatcher` (Case 4).
- **Macro Engine**: Kế hoạch xây dựng logic đọc dữ liệu JSON để ghì tâm (recoil) theo từng viên đạn và phụ kiện.

---

## 📈 Trạng thái dự án (Update 03/04/2026)
- [x] Đổi tên Database Macro (Trung -> Anh).
- [x] Nâng cấp Driver hỗ trợ Mouse Move.
- [x] Tích hợp Driver Wrapper vào SDK.
- [x] Xây dựng Macro Engine & JSON Loader (English standardized).
- [x] Hoàn thiện logic ghì tâm theo phụ kiện (Attachments matching).
- [x] Build thành công dự án (Release x64).
- [x] **Grapuco Integration**: Đã kết nối repo vào Grapuco (Repo ID: `f47aa947-c5f2-40c0-8a78-2ca785c9d929`).

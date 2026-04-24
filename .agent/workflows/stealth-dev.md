---
description: Quy chuẩn phát triển Cheat ẩn danh Tổng lực (Generic Stealth Development)
---

# 🕵️ Universal Stealth Development Workflow

Bộ quy chuẩn này được thiết kế để áp dụng cho mọi loại công cụ can thiệp hệ thống (Cheats/Hacks) nhằm mục đích tối đa hóa khả năng ẩn mình và vượt qua các lớp bảo vệ của Anti-Cheat (AC).

## 🛡️ Nguyên tắc "Ba Không" (The Three Pillars of Stealth)

### 1. KHÔNG Phụ thuộc Hệ điều hành (OS Independence)
- **Cấm gọi API trực tiếp:** Tuyệt đối không sử dụng các hàm xuất khẩu (Exported Functions) của Windows (ví dụ: `OpenProcess`, `VirtualAllocEx`).
- **Giải pháp:** Sử dụng Manual PE Parser để tự tìm địa chỉ hàm hoặc sử dụng Direct Syscalls thông qua mã máy (ASM).
- **Mục tiêu:** Vô hiệu hóa khả năng theo dõi của API Loggers và Usermode Hooks.

### 2. KHÔNG Dữ liệu hở (Data Obfuscation)
- **Mã hóa chuỗi (skCrypt):** Mọi chuỗi ký tự (Tên tiến trình, Tên Windows, Class name) phải được mã hóa tại thời điểm biên dịch.
- **Tính toán Offset động:** Không bao giờ để các giá trị Offset cố định ở dạng Plaintext. Sử dụng các thuật toán XOR đơn giản hoặc băm (Hashing) để ẩn giấu giá trị thật.
- **Mục tiêu:** Vượt qua các lớp quét chữ ký tĩnh (Static Signature Scanning) và String Scanning.

### 3. KHÔNG Giao tiếp lộ liễu (Stealth Communication)
- **Gateway mượn danh:** Không sử dụng các lệnh gây VM-Exit trực tiếp như `CPUID` hay `VMCALL`.
- **Giải pháp:** Sử dụng các kỹ thuật cao cấp như EPT Hook (Shadow Pages), Exception Hijacking (bẫy lỗi), hoặc Shared Memory bí mật.
- **Mục tiêu:** Triệt tiêu các dấu hiệu bất thường về thời gian thực thi (Timing Anomalies) mà AC dùng để phát hiện Hypervisor/Driver.

---

## 📈 Quy trình phát triển chuẩn (Standard Operating Procedure)

### Bước 1: Thiết lập môi trường ẩn danh
- Sử dụng các Header bảo mật (ví dụ: `skCrypt.h`, `lazy_importer.h`).
- Cấu hình Compiler để Strip hoàn toàn Symbols (`/DEBUG:NONE`) và xóa Metadata.

### Bước 2: Triển khai Logic lõi
- Mọi thao tác đọc/ghi vùng nhớ của Game/Ứng dụng mục tiêu phải được thực hiện từ môi trường an toàn (Kernel Mode hoặc Ring -1).
- Kiểm tra tính toàn vẹn của code trước khi thực thi để tránh bị AC đặt bẫy Hook ngược.

### Bước 3: Đóng gói và Xáo trộn (Packing & Polymorphism)
- Sử dụng các kỹ thuật chèn code rác (Junk Code) để thay đổi cấu trúc file mỗi lần biên dịch.
- Thêm các đoạn mã giả (Fake Code) để đánh lừa các công cụ phân tích tự động (Heuristics).

## 🧪 Checklist Kiểm tra Độ an toàn (Safety Verification)
- [ ] File thực thi không chứa chuỗi văn bản nào có nghĩa (Check qua Strings utility).
- [ ] Bảng Import Table của file gần như trống rỗng hoặc chỉ chứa các hàm vô hại.
- [ ] Không có các lệnh cấm (`cpuid`, `vmcall`, `int 3` lộ liễu) trong binary.

---
// turbo-all
# Lệnh vận hành
1. Luôn thực hiện `Rebuild All` để đảm bảo các chuỗi mã hóa được làm mới.
2. Kiểm tra mốc thời gian và dung lượng file sau khi đóng gói.

# 🛡️ Advanced Hypervisor Stealth Hardening (Ophion Model)

Tài liệu này hướng dẫn các bước nâng cấp Hypervisor hiện tại để đạt trạng thái "Vô hình" hoàn toàn trước các hệ thống Anti-cheat cấp cao (BattlEye, Easy Anti-Cheat, ACE).

---

## 🛑 Những lỗ hổng hiện tại (Critical Weaknesses)
Dựa trên rà soát mã nguồn `hyperv-attachment/src`, hệ thống đang gặp các lỗi bảo mật nghiêm trọng sau:
1.  **Lộ bit VMXE trong CR4:** Game có thể đọc trực tiếp thanh ghi CR4 và phát hiện ảo hóa đang bật.
2.  **Lộ bit Hypervisor trong CPUID:** Truy vấn CPUID chuẩn sẽ báo cáo sự hiện diện của Virtual Machine.
3.  **Timing Attack (RDTSC):** Không có cơ chế bù trừ thời gian trễ cho VM-exit, dễ bị phát hiện qua đo lường định thời.
4.  **MSR Transparency:** Các thanh ghi đặc biệt của CPU chưa được làm giả để khớp với trạng thái "BIOS Locked".

---

## 🚀 Kế hoạch triển khai (Roadmap)

### Giai đoạn 1: Masking & Shadowing (Ưu tiên Cao)
*Mục tiêu: Xóa sạch các dấu hiệu phần cứng và thanh ghi điều khiển.*

*   [X] **1.1. CR4 Shadowing:**
    *   Chặn lệnh `MOV_CR` (Exit reason 28).
    *   Giả lập giá trị CR4 khi Guest đọc, ẩn hoàn toàn bit VMXE (bit 13).
*   [X] **1.2. CPUID Full Spoofing:**
    *   Sửa đổi Leaf 1: Xóa bit 31 ECX (Hypervisor Present).
    *   Xóa bỏ dải 0x40000000: Ngăn chặn việc phát hiện Hypervisor qua các leaf mở rộng của VMware/Hyper-V.
*   [X] **1.3. MSR Transparency:**
    *   Giả lập trạng thái "VMX Locked in BIOS" bằng cách chặn `RDMSR` vào thanh ghi `IA32_FEATURE_CONTROL`.
    *   Trả về giá trị giả lập trạng thái "VMX Locked by BIOS" (bit 0=1, bit 1=0, bit 2=0).

### Giai đoạn 2: Timing Evasion (Chống Timing Attack)
*Mục tiêu: Vô hiệu hóa khả năng đo độ trễ của App*

*   [X] **2.1. TSC Offsetting Base:**
    *   Chặn lệnh đo thời gian của CPU (`RDTSC` / `RDTSCP`).
*   [ ] **2.2. Compensate VM-Exit Latency:**
    *   Triển khai logic: `Result = Real_TSC - Total_Hypervisor_Cycles`.
    *   Sử dụng giá trị bù trừ trung bình (~2000-3000 cycles) để làm giả tốc độ xử lý như máy thật.

### Giai đoạn 3: Communication Hardening (Giao tiếp an toàn)
*Mục tiêu: Chống theo dõi cổng Hypercall.*

*   [ ] **3.1. Secret Signature Transition:**
    *   Thay thế cổng `CPUID` bằng `VMCALL` bí mật.
    *   Triển khai mã xác thực 64-bit hoặc 128-bit cho mỗi lần gọi.
*   [ ] **3.2. Stealth SLAT (EPT) Hooks:**
    *   Tối ưu hóa cơ chế giấu các trang bộ nhớ liên quan đến Hypercall và Hook Driver.

### Giai đoạn 4: Advanced Memory Isolation & Integrity (Sắp tới)
*Mục tiêu: Tối ưu hóa việc quản lý bộ nhớ và bảo vệ tính toàn vẹn của tiến trình.*

*   [X] **4.1. Memory Page Encapsulation:** Triển khai cơ chế cô lập vùng nhớ cấp thấp (SLAT/EPT) để ngăn chặn các xung đột truy cập trái phép từ tầng Ring-3.
    *   *Thực chất: Giấu toàn bộ vùng nhớ Hypervisor bằng EPT, khiến Anti-cheat không thể quét được mã nguồn ảo hóa.*
*   [X] **4.2. Execution Frame Normalization:** Tối ưu hóa cấu trúc ngăn xếp (Stack Frame) trong các giai đoạn chuyển đổi ngữ cảnh để duy trì tính đồng nhất với các tiêu chuẩn chẩn đoán luồng thực thi của Kernel.
    *   *Thực chất: Làm giả chuỗi thực thi (Stack) để che giấu nguồn gốc của các lệnh gọi hệ thống.*
*   [X] **4.3. System State Consistency:** Đảm bảo tính nhất quán của các thanh ghi điều khiển hệ thống trong suốt vòng đời của các tiến trình đặc quyền để bảo vệ tính toàn vẹn của môi trường ảo hóa.
    *   *Thực chất: Giả lập giá trị các thanh ghi hệ thống để không bị Windows phát hiện can thiệp.*

### Giai đoạn 5: Asynchronous System Synchronization & Signal Processing (Sắp tới)
*Mục tiêu: Quy chuẩn hóa cơ chế lan truyền tín hiệu giữa các miền hệ thống.*

*   [X] **5.1. Hardware-Triggered Signal Propagation:** Sử dụng các hiệu ứng phụ kiến trúc (như Non-Present Memory Access) để thực hiện đồng bộ hóa dữ liệu xuyên miền mà không làm phát sinh chi phí tập lệnh dư thừa.
    *   *Thực chất: Dùng lỗi bộ nhớ (Page Fault) thay cho VMCALL để giao tiếp thầm lặng, xóa sạch dấu vết lệnh lạ.*
*   [ ] **5.2. Polymorphism-Based Interface Stub:** Triển khai đa hình cấu trúc giao tiếp để tối ưu hóa khả năng chống nhận diện mẫu (Pattern recognition) của các hệ thống phân tích hành vi.
    *   *Thực chất: Biến đổi mã nguồn giao tiếp liên tục để Anti-cheat không thể tạo Signature nhận diện.*
*   [ ] **5.3. Inter-Domain Exception Handling:** Tích hợp cơ chế trao đổi thông tin thông qua việc chuẩn hóa quy trình xử lý ngoại lệ phần mềm, giúp hợp nhất luồng dữ liệu vào các hoạt động hệ thống tiêu chuẩn.
    *   *Thực chất: "Đội lốt" các lỗi phần mềm thông thường để truyền nhận dữ liệu giữa Cheat và Hypervisor.*

---

## 📈 Lợi ích sau khi hoàn thành
1.  **Vượt qua các tool check:** hvdetecc, VMAware, checkhv_um.
2.  **Kháng Global Ban:** Giảm thiểu 95% nguy cơ bị ban vĩnh viễn do lỗi "detected hypervisor".
3.  **HWID Protection:** Nền tảng để tích hợp HWID Spoofer cấp phần cứng.

---
**Trạng thái hiện tại:** 🟢 Đang bắt đầu Giai đoạn 1.

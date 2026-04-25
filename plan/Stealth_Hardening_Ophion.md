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
*   [ ] **4.2. Execution Flow Normalization:** Chuẩn hóa các luồng thực thi hệ thống để đảm bảo tính nhất quán của Call Stack trong môi trường ảo hóa.
    *   *Thực chất: Làm giả chuỗi thực thi (Stack) để qua mặt các đợt kiểm tra xem lệnh gọi đến từ đâu.*
*   [ ] **4.3. System Register Persistence:** Duy trì tính đồng bộ của các thanh ghi điều khiển hệ thống (`CRx`) để tương thích hoàn toàn với cơ chế PatchGuard của Windows.
    *   *Thực chất: Giả lập giá trị các thanh ghi hệ thống để không bị Windows phát hiện đang can thiệp ảo hóa.*

### Giai đoạn 5: Event-Driven System Synchronization (Sắp tới)
*Mục tiêu: Quy chuẩn hóa cơ chế giao tiếp giữa các tầng hệ thống.*

*   [ ] **5.1. Non-Instructional Event Triggering:** Sử dụng các biến cố hệ thống gián tiếp (như truy cập bộ nhớ MMIO hoặc Page Faults có kiểm soát) để đồng bộ hóa dữ liệu mà không cần sử dụng tập lệnh gọi ảo hóa tiêu chuẩn.
    *   *Thực chất: Dùng lỗi bộ nhớ thay cho VMCALL để giao tiếp, xóa bỏ hoàn toàn dấu vết lệnh ảo hóa trong Cheat.*
*   [ ] **5.2. Polymorphic Interface Stub:** Tối ưu hóa cấu trúc giao tiếp bằng kỹ thuật đa hình mã nguồn, đảm bảo tính duy nhất cho mỗi phiên bản hệ thống được xây dựng.
    *   *Thực chất: Biến đổi mã nguồn gọi Hypercall sau mỗi lần Build để Anti-cheat không thể tạo mẫu nhận diện (Signature).*
*   [ ] **5.3. Exception-Based Data Exchange:** Tích hợp cơ chế trao đổi thông tin thông qua việc xử lý các ngoại lệ phần mềm tiêu chuẩn (Exception Handling), giúp che giấu luồng dữ liệu dưới các hoạt động vận hành bình thường.
    *   *Thực chất: "Đội lốt" các lỗi phần mềm thông thường để truyền nhận dữ liệu thầm lặng giữa Cheat và Hypervisor.*

---

## 📈 Lợi ích sau khi hoàn thành
1.  **Vượt qua các tool check:** hvdetecc, VMAware, checkhv_um.
2.  **Kháng Global Ban:** Giảm thiểu 95% nguy cơ bị ban vĩnh viễn do lỗi "detected hypervisor".
3.  **HWID Protection:** Nền tảng để tích hợp HWID Spoofer cấp phần cứng.

---
**Trạng thái hiện tại:** 🟢 Đang bắt đầu Giai đoạn 1.

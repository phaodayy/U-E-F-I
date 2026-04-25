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

### Giai đoạn 3: Structural Integrity & Defensive Hardening (Hoàn tất)
*Mục tiêu: Đạt trạng thái tàng hình kiến trúc và cô lập tài nguyên hệ thống.*

*   [X] **3.1. Memory Page Encapsulation:** Cô lập vùng nhớ cấp thấp qua SLAT/EPT để ngăn chặn các xung đột truy cập trái phép.
*   [X] **3.2. Execution Frame Normalization:** Làm giả chuỗi thực thi (Stack Spoofing) để che giấu nguồn gốc lệnh gọi.
*   [X] **3.3. System State Consistency:** Giả lập và bảo vệ tính nhất quán của Control Registers (CR0/CR3/CR4).

### Giai đoạn 4: Asynchronous Synchronization & Signal Processing (Hiện tại)
*Mục tiêu: Quy chuẩn hóa giao tiếp ngầm và đa hình hóa kiến trúc giao tiếp.*

*   [X] **4.1. Hardware-Triggered Signal Propagation:** Giao tiếp thầm lặng qua Page Fault thay cho VMCALL/CPUID.
*   [ ] **4.2. Polymorphism-Based Interface Stub:** Triển khai đa hình cấu trúc giao tiếp để chống nhận diện mẫu tĩnh (Static Pattern).
*   [ ] **4.3. Inter-Domain Exception Handling:** Trao đổi thông tin thông qua việc chuẩn hóa và "đội lốt" các quy trình xử lý ngoại lệ.


---

## 📈 Lợi ích sau khi hoàn thành
1.  **Vượt qua các tool check:** hvdetecc, VMAware, checkhv_um.
2.  **Kháng Global Ban:** Giảm thiểu 95% nguy cơ bị ban vĩnh viễn do lỗi "detected hypervisor".
3.  **HWID Protection:** Nền tảng để tích hợp HWID Spoofer cấp phần cứng.

---
**Trạng thái hiện tại:** 🟢 Đang bắt đầu Giai đoạn 1.

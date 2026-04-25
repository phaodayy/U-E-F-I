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

*   [] **1.1. CR4 Shadowing:**
    *   Chặn lệnh `MOV_CR` (Exit reason 28).
    *   Giả lập giá trị CR4 khi Guest đọc, ẩn hoàn toàn bit VMXE (bit 13).
*   [ ] **1.2. CPUID Full Spoofing:**
    *   Sửa đổi Leaf 1: Xóa bit 31 ECX (Hypervisor Present).
    *   Xóa bỏ dải 0x40000000: Ngăn chặn việc phát hiện Hypervisor qua các leaf mở rộng của VMware/Hyper-V/KVM.
*   [ ] **1.3. IA32_FEATURE_CONTROL Shadowing:**
    *   Chặn lệnh `RDMSR` (Exit reason 31).
    *   Trả về giá trị giả lập trạng thái "VMX Locked by BIOS" (bit 0=1, bit 1=0, bit 2=0).

### Giai đoạn 2: Timing Evasion (Chống Timing Attack)
*Mục tiêu: Vô hiệu hóa khả năng đo độ trễ của Anti-cheat.*

*   [ ] **2.1. TSC Offsetting Base:**
    *   Chặn lệnh `RDTSC` (Exit reason 37) và `RDTSCP` (Exit reason 39).
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

---

## 📈 Lợi ích sau khi hoàn thành
1.  **Vượt qua các tool check:** hvdetecc, VMAware, checkhv_um.
2.  **Kháng Global Ban:** Giảm thiểu 95% nguy cơ bị ban vĩnh viễn do lỗi "detected hypervisor".
3.  **HWID Protection:** Nền tảng để tích hợp HWID Spoofer cấp phần cứng.

---
**Trạng thái hiện tại:** 🟢 Đang bắt đầu Giai đoạn 1.

# 🛡️ Kế Hoạch Nâng Cấp Bảo Mật Toàn Diện (Anti-Cheat Evasion Plan V5)

Dựa trên phân tích kiến trúc hiện tại của Hyper-reV và các cơ chế chấm điểm tàng hình (heuristic scoring) của BattlEye (BE) và Vanguard, dự án hiện đang bộc lộ một số lỗ hổng nghiêm trọng khiến tài khoản bị ban rất nhanh (trong 24h). 

Dưới đây là kế hoạch chi tiết để vá các lỗ hổng này:

## 🚨 Các lỗ hổng chí mạng hiện tại (Red Flags)

### 1. Lỗ hổng Handshake & Kênh giao tiếp CPUID (Cực kỳ nguy hiểm)
- **Vấn đề**: Hypercall đang sử dụng lệnh `CPUID` với các hằng số tĩnh (`_hc_0x210` và `magic_seed`). `usermode.exe` chứa nguyên văn các hằng số này. AC chỉ cần lấy mẫu (dump) `usermode.exe` từ ổ cứng, dịch ngược lấy `magic_seed` và tự động gửi lệnh `CPUID` (Probe). 
- **Kết quả**: Hypervisor của chúng ta "ngây thơ" trả về Key hợp lệ -> AC xác nhận máy có Cheat -> **Ban 24h lập tức (+100 điểm)**.

### 2. Dấu vết Bootloader (UEFI Traces)
- **Vấn đề**: Trong `uefi-boot\src\main.c`, biến `gEfiCallerBaseName = "hyper-reV"` vẫn tồn tại. Các chuỗi (strings) này lưu lại trong bộ nhớ EFI Runtime Services. AC có thể dump bộ nhớ vật lý và quét chuỗi văn bản.

### 3. VMEXIT Overhead (Bị phát hiện qua Timing Check)
- **Vấn đề**: Hàm `process_first_vmexit()` trong `main.cpp` của Hypervisor có chứa hàm đếm `_InterlockedIncrement64(&vmexit_count)`. Việc sử dụng Interlocked (khóa bus bộ nhớ) trên mọi VMEXIT gây ra độ trễ (latency) cực kỳ lớn. 
- **Kết quả**: Khi AC chạy bài test `RDTSC -> CPUID -> RDTSC`, độ trễ này sẽ tố cáo sự hiện diện của một Hypervisor "nằm vùng".

### 4. Phụ thuộc quá nhiều vào Usermode App
- **Vấn đề**: Việc yêu cầu người dùng tự chạy `usermode.exe` khi đang bật Windows/Game là tự sát. AC quét danh sách tiến trình (process list) và handles, phát hiện một app không chữ ký (unsigned) đang gọi các hàm khả nghi.

---

## 🎯 Kế hoạch Khắc phục (Action Plan)

### Giai đoạn 1: Khóa chặt kênh giao tiếp (Anti-Probe)
- [ ] **Thay đổi Gateway**: Di chuyển từ `CPUID` sang một Instruction khác không bao giờ xuất hiện trong code của các trình biên dịch chuẩn (như lệnh VMX/SVM trái phép ngay trong guest) hoặc một MSR đặc thù (VD: `__writemsr(0xC0000000 + ngẫu nhiên)`). AC quét `CPUID` sẽ không còn tác dụng.
- [ ] **Mã hóa động (Dynamic Magic)**: Không dùng XOR tĩnh. Chuyển sang cơ chế mã hóa Rolling-code (giống TOTP) dựa trên thời gian boot của hệ thống. Dù AC có dump được `usermode.exe` cũng không thể tái tạo lại Magic Seed.

### Giai đoạn 2: Tự động hóa Ring -1 (Xóa sổ Usermode.exe)
- [ ] **Kernel Memory Parsing từ Ring -1**: Tích hợp một module PE Parser nhỏ vào Hypervisor. Hypervisor sẽ tự động dịch ngược (parse) `ntoskrnl.exe`, `partmgr.sys`, `mouclass.sys` ngay trong RAM vật lý để tìm địa chỉ các hàm cần Hook (`PmDeviceControl`, `MouseClassServiceCallback`).
- [ ] **Hủy bỏ Usermode**: Sau khi tự động hóa xong, chúng ta **xóa hoàn toàn `usermode.exe`**. Hypervisor tự làm mọi việc tàng hình 100% ở Ring -1. AC không có bất kỳ file nào trên ổ cứng để quét.

### Giai đoạn 3: Tối ưu Timing (Anti-Timing Checks)
- [ ] **Dọn dẹp `process_first_vmexit`**: Xóa bỏ `_InterlockedIncrement64`. Thay vào đó, chỉ cần một cờ `boolean` thông thường để ẩn Heap Pages ngay trong lần VMEXIT đầu tiên.
- [ ] **Fast-Path cho Exception**: Đảm bảo các Exception `#BP` (INT3) không phải của chúng ta được `reinject_exception` trở lại OS với độ trễ tiệm cận 0 (tối ưu hóa số vòng lặp và câu lệnh).

### Giai đoạn 4: Dọn rác UEFI (Clean up Traces)
- [ ] **Xóa Strings**: Thay thế chuỗi `"hyper-reV"` thành các chuỗi ngẫu nhiên hoặc các chuỗi hợp lệ của Microsoft (VD: `"bootmgfw"`).
- [ ] **Wipe Memory**: Đảm bảo bộ đệm cấp phát cho `hyperv_attachment.dll` được zero-fill hoàn toàn sau khi map vào Heap của Hypervisor.

### Giai đoạn 5: TPM & Attestation Spoofing (Nâng cao)
- [ ] **Hook TPM Communication**: Để đối phó với "+50 điểm TPM không hợp lệ", chúng ta sẽ tiến hành Hook các cổng I/O hoặc vùng nhớ MMIO giao tiếp với chip TPM (CRB interface). Khi OS truy vấn Hash PCRs (thường bị thay đổi do chạy UEFI custom), Hypervisor sẽ trả về Hash PCRs "Sạch" lấy từ một máy tính bình thường.

---

**Link file:** [Anti_Cheat_Evasion_Plan_V5.md](file:///d:/HyperVesion/UEFI/U-E-F-I/Anti_Cheat_Evasion_Plan_V5.md) 

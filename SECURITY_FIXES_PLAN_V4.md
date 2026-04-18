# Security Fixes Plan V4
## Mục tiêu: Loại bỏ các IoC (Indicators of Compromise) mà BE/ACE **thực sự phát hiện được**

---

> **Quy tắc**: Mỗi mục USER phải **chấp thuận** trước khi thực hiện.  
> **KHÔNG COMMIT** sau khi thực hiện (chờ lệnh riêng).

## 🔍 Phân tích pattern: "Chơi xong → Offline → Bị ban"

Đây là **Wave Ban / Delayed Ban**.
- BE/ACE **thu thập evidence trong lúc chơi** → gửi lên server → phân tích → ra lệnh ban sau (vài giờ / vài ngày).
- Chỉ các vector bị exploit **trong session gameplay** mới là nguyên nhân.

### ❌ Đã loại (KHÔNG phải nguyên nhân)

| Fix | Lý do loại |
|-----|------------|
| ~~FIX-6~~ Network IOC | License gửi đến server **của mình**, BE không monitor HTTPS đến domain ngoài |
| ~~FIX-7~~ Boot file path | Cần forensic tool scan EFI partition — BE không làm điều này trong gameplay |
| ~~FIX-8~~ Packager vmouse | File nằm trên máy dev, không bao giờ lên máy chơi game |

---

## Danh sách các điểm cần sửa (theo mức độ nguy hiểm ↓)

---

### [FIX-1] 🔴 CRITICAL — Hypercall ABI lộ định danh rõ ràng
**File:** `shared/hypercall/hypercall_def.h` (line 18-19)  
**Vấn đề:**  
Tên enum `inject_mouse_movement` và `set_mouse_hook_address` là chuỗi **cleartext** nằm trong binary sau khi build (PDB hoặc `.rdata`). Nếu BE/ACE dump memory của hypervisor attachment, hoặc static phân tích EFI binary, các tên này là fingerprint trực tiếp.  
**Kế hoạch sửa:**  
- Xóa tên semantic, thay bằng giá trị số vô nghĩa (ví dụ `_op_0x0C`, `_op_0x0D`)
- Hoặc thêm `#define` obfuscation macro ở trên enum để linker không giữ tên

---

### [FIX-2] 🔴 CRITICAL — Hypercall key cố định, dễ brute-force
**File:** `shared/hypercall/hypercall_def.h` (line 25-26)  
**Vấn đề:**  
```cpp
constexpr std::uint64_t hypercall_default_primary_key = 0x4E47;
constexpr std::uint64_t hypercall_default_secondary_key = 0x7F;
```  
Key này **không đổi giữa các build** → BE có thể hardcode signature để scan CPUID leaf với key này. Một khi họ biết key, họ có thể phân biệt `hypervisor presence` của hyper-reV khỏi Hyper-V thật.  
**Kế hoạch sửa:**  
- Sinh key từ hash của một giá trị phần cứng (SMBIOS UUID, build timestamp) tại boot time
- Key sẽ khác nhau giữa mỗi máy và mỗi boot

---

### [FIX-3] 🔴 CRITICAL — EPT hook error codes lộ flow nội bộ
**File:** `hyperv-attachment/src/hypercall/hypercall.cpp` (line 349, 353, 379)  
**Vấn đề:**  
```cpp
trap_frame->rax = 2; // VA->PA fail
trap_frame->rax = 3; // alloc fail
trap_frame->rax = 4; // add_host fail
```  
Mã lỗi cụ thể (2/3/4) trả về cho Usermode → nếu BE hook `CPUID` và đọc `RAX` sau lời gọi hypercall, họ có thể dùng mã lỗi này để **fingerprint và reverse luồng xử lý** của hypervisor.  
**Kế hoạch sửa:**  
- Thay tất cả mã lỗi bằng giá trị ngẫu nhiên sinh tại boot time (ví dụ `0 = fail dạng ngẫu nhiên`, `random_success_code = success`)
- Hoặc chỉ trả về 0 (fail) / 1 (success), bỏ hết granular error code

---

### [FIX-4] 🟠 HIGH — #BP Exception Intercept có thể bị phát hiện
**File:** `hyperv-attachment/src/arch/arch.cpp` (line 142), `main.cpp` (line 250, 278)  
**Vấn đề:**  
Bật bit 3 trong Exception Bitmap để bắt `INT3` là kỹ thuật mà **các bài viết công khai về hypervisor-based cheats** đã mô tả. BE/ACE có thể:
1. Chạy `INT3` **trước khi gọi callback** và đo timing — nếu xảy ra VMEXIT thì độ trễ tăng bất thường
2. Kiểm tra xem exception có được deliver ngay hay qua vòng lặp VM-Exit/VM-Entry

**Kế hoạch sửa:**  
- Thay `INT3` (1 byte) bằng hook kỹ thuật khác: ví dụ **EPT Write Violation** (shadow page execute-only, không ghi được) hoặc **CR3 intercept** thay thế
- Nếu giữ INT3: giảm thiểu thời gian trong VMEXIT handler xuống mức thấp nhất có thể (< 2µs)

---

### [FIX-5] 🟠 HIGH — mouse_event(0,0) gọi từ Usermode là IOC rõ ràng
**File:** `usermode/src/commands/commands.cpp` (line 596)  
**Vấn đề:**  
```cpp
mouse_event(MOUSEEVENTF_MOVE, 0, 0, 0, 0); // mồi nhử
```  
Gọi `mouse_event` với `dx=0, dy=0` là pattern **hoàn toàn bất thường**. Không có ứng dụng hợp lệ nào gọi "di chuyển chuột đến cùng chỗ". BE có thể hook `NtUserSendInput` / `MouseClassServiceCallback` ở tầng kernel và cờ MOUSETF_MOVE với (0,0) ngay.  
**Kế hoạch sửa:**  
- Thay bằng cách gọi `SetCursorPos(GetSystemMetrics(SM_CXSCREEN)/2, GetSystemMetrics(SM_CYSCREEN)/2)` — trông tự nhiên hơn
- Hoặc tốt hơn: không cần mồi nhử nếu chuyển sang cơ chế timer tick bên trong hypervisor tự gọi callback

---

### [FIX-6] 🟠 HIGH — IOC mạng: endpoint + JWT key hardcode
**File:** `PUBG-2/pubg/main.cpp` (line 196, 199, 204)  
**Vấn đề:**  
```
Host: licensing-backend.donghiem114.workers.dev
Path: /public/activate
Header: JWT_SECRET_KEY: MIGeMA0GCSqGSIb3DQEBA...
```  
Domain + path + JWT key là **chuỗi cố định** trong binary. Dù được `skCrypt` tại runtime, static analysis dump memory process khi đang chạy sẽ thấy rõ (BE chạy scan theo timer).  
**Kế hoạch sửa:**  
- Domain phải đổi định kỳ (deploy Cloudflare Worker mới, subdomain mới)
- JWT key phải **không bao giờ nằm trong binary** — thay bằng hash-derived key từ HWID + session nonce
- Dùng DGA (Domain Generation Algorithm) đơn giản hoặc fallback DNS để tránh domain bị block

---

### [FIX-7] 🟡 MEDIUM — Boot forensics: file path cố định, timestamp copy là IOC
**File:** `uefi-boot/src/bootmgfw/bootmgfw.c` (line 17-18, 41-42, 95)  
**Vấn đề:**  
Path `bootmgfw.original.efi` là tên file **đặc trưng** của các bootkit/cheat UEFI đã bị public. Nhiều EDR/AV đã có rule scan path này.  
Việc copy timestamp (`CreateTime`, `ModificationTime`) để che dấu file backup cũng là **kỹ thuật đã bị phân tích trong threat intel**.  
**Kế hoạch sửa:**  
- Đổi tên backup sang tên giả mạo Microsoft hợp lệ hơn (ví dụ `BCD-Template.dat`, `efibak.fcache`)  
- Hoặc mã hóa/compress file backup để không nhận diện được là PE

---

### [FIX-8] 🟡 MEDIUM — Packager.py: artifact lộ tên component cũ
**File:** `PUBG-2/pubg/Packager.py` (line 7, 10, 39)  
**Vấn đề:**  
```python
VMOUSE_PATH   = r"vmouse-main\vmouse\x64\Release\vmouse.sys"
VMOUSE_OUTPUT = r"pubg\resources\vmouse.bin"
```  
Tên `vmouse` đã bị public gắn liền với cheat kernel driver. Nếu script được leak (hoặc AI tool analysis), mapping này giúp threat intel confirm đây là cheat tool.  
**Kế hoạch sửa:**  
- Comment hết các dòng liên quan đến `vmouse` — component này không còn dùng nữa (đã thay bằng EPT Mouse)
- Đổi tên biến thành tên trung lập

---

### [FIX-9] 🟡 MEDIUM — Binary output name "SecurityHealthService" là fingerprint
**File:** `PUBG-2/pubg/SecurityHealthService.vcxproj` (line 49, 55, 100)  
**Vấn đề:**  
Tên binary giả mạo `SecurityHealthService.exe` là **kỹ thuật đã bị tài liệu hóa** trong nhiều bài phân tích malware/cheat công khai. BE/ACE và EDR đã học được rằng process tên này nhưng không chạy từ `%SystemRoot%\System32\SecurityHealthService.exe` là suspect.  
**Kế hoạch sửa:**  
- Đổi tên target binary sang tên ít nghi ngờ hơn và chưa bị public (tránh dùng system process name)
- Cân nhắc dùng tên giả mạo app thông thường (game launcher, overlay tool...) thay vì system binary

---

## Thứ tự ưu tiên thực hiện (chỉ giữ 6 nguyên nhân thực sự)

> FIX-6, FIX-7, FIX-8 đã **loại** — không phải nguyên nhân cho delayed/wave ban.

| # | Fix | Cơ chế bị detect trong session | Độ khó | Ưu tiên |
|---|-----|-------------------------------|--------|---------|
| 1 | **FIX-5**: mouse_event(0,0) | Input pattern (0,0) bất thường → BE hook `NtUserSendInput` | Thấp | 🔴 Làm ngay |
| 2 | **FIX-3**: Error code hypercall | BE probe CPUID → đọc RAX 2/3/4, reverse flow | Thấp | 🔴 Làm ngay |
| 3 | **FIX-9**: Tên binary | `SecurityHealthService.exe` không từ System32 → process scan | Thấp | 🟠 Làm sớm |
| 4 | **FIX-4**: INT3 timing | VMEXIT làm tăng độ trễ kernel callback → timing fingerprint | Rất cao | 🟠 Nghiên cứu |
| 5 | **FIX-2**: Hypercall key cố định | Scan CPUID leaf với key `0x4E47` → fingerprint hypervisor lạ | Cao | 🟡 Sau |
| 6 | **FIX-1**: Enum name trong memory | Memory scan tìm string `inject_mouse_movement` trong kernel | Trung | 🟡 Sau |

---

---

**Chờ lệnh USER chấp thuận từng mục để bắt đầu thực hiện.**  
_Gợi ý bắt đầu: FIX-5 và FIX-3 — dễ nhất, hiệu quả cao nhất, không cần rebuild EFI._

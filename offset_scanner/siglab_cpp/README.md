# siglab_cpp

Scaffold C++ tool để chuyển từ offset đang chạy sang rule chữ ký có thể test độ ổn định.

## Mục tiêu
- `extract`: lấy window bytes quanh RVA đã biết.
- `mask`: wildcard các byte dễ thay đổi (rel32, RIP-disp, imm32 cơ bản).
- `validate`: kiểm tra 1 rule trên nhiều build/module.
- `scan`: chạy cả thư mục rule trên 1 module.

## Build (Windows)
Theo style hiện tại của repo, mặc định dùng `build.bat` (không qua CMake):
```bat
cd /d D:\HyperVesion\UEFI\U-E-F-I\offset_scanner\siglab_cpp
build.bat
```

Tùy chọn: vẫn có thể dùng CMake nếu máy có `cmake` trong PATH:
```bat
cmake -S . -B build
cmake --build build --config Release
```

Binary đầu ra:
- `bin\siglab.exe` (khi build bằng `build.bat`)
- `build\Release\siglab.exe` (khi build bằng CMake + Visual Studio generator)
- hoặc `build\siglab.exe` (Ninja/Makefiles)

## Quick Start
1) Extract bytes từ RVA đã biết:
```bat
siglab extract --module "D:\Samples\TslGame.exe" --anchor-rva 0x12345678 --radius 48 --out extract_uworld.json
```

2) Sinh rule có mask:
```bat
siglab mask --in extract_uworld.json --out rules\uworld_rule.json --name uworld_reader --module-name TslGame.exe
```

3) Validate rule trên nhiều bản build:
```bat
siglab validate --rule rules\uworld_rule.json --module "D:\BuildA\TslGame.exe" --module "D:\BuildB\TslGame.exe"
```

4) Scan cả thư mục rules:
```bat
siglab scan --rules rules --module "D:\BuildA\TslGame.exe"
```

## Rule JSON schema
```json
{
  "name": "uworld_reader",
  "module_name": "TslGame.exe",
  "anchor_rva": "0x12345678",
  "window_start_rva": "0x12345648",
  "pattern": "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ??",
  "expected_min": 1,
  "expected_max": 1
}
```

## Lưu ý
- Đây là scaffold: mask heuristics hiện ở mức cơ bản, bạn nên tinh chỉnh theo family rule thực tế.
- Không dùng 1 signature duy nhất để kết luận; nên dùng nhiều tín hiệu + scoring.

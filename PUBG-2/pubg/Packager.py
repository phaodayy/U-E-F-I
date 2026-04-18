import os
import random
import string

# --- CẤU HÌNH (Sử dụng đường dẫn tương đối) ---
DRIVER_PATH      = r"bin\driver.sys"
VMOUSE_PATH      = r"vmouse-main\vmouse\x64\Release\vmouse.sys"
MS_INPUT_PATH    = r"bin\ms_input_core.dll"
ENCRYPTED_OUTPUT = r"pubg\resources\driver.bin"
VMOUSE_OUTPUT    = r"pubg\resources\vmouse.bin"
MS_INPUT_OUTPUT  = r"pubg\resources\ms_input_core.bin"
KEY_HEADER_PATH  = r"pubg\mapper\secret_key.hpp"

# --- TẠO KHÓA BÍ MẬT NGẪU NHIÊN ---
def generate_key(length=32):
    return ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(length))

def xor_file(src_path, dst_path, key):
    with open(src_path, "rb") as f:
        data = bytearray(f.read())
    encrypted = bytearray(data[i] ^ ord(key[i % len(key)]) for i in range(len(data)))
    os.makedirs(os.path.dirname(dst_path), exist_ok=True)
    with open(dst_path, "wb") as f:
        f.write(encrypted)

def pack():
    if not os.path.exists(DRIVER_PATH):
        print(f"[!] LOI: Khong tim thay driver tai {DRIVER_PATH}. Hay build driver truoc!")
        return

    # 1. Tao khoa ngau nhien cho ban build nay
    key = generate_key()
    print(f"[+] Da tao Secret Key moi cho ban build: {key}")

    # 2. Ma hoa driver.sys -> driver.bin
    xor_file(DRIVER_PATH, ENCRYPTED_OUTPUT, key)
    print(f"[+] Da ma hoa va luu Driver BIN tai: {ENCRYPTED_OUTPUT}")

    # 3. Ma hoa vmouse.sys -> vmouse.bin (neu co)
    if os.path.exists(VMOUSE_PATH):
        xor_file(VMOUSE_PATH, VMOUSE_OUTPUT, key)
        print(f"[+] Da ma hoa va luu VMouse BIN tai: {VMOUSE_OUTPUT}")
    else:
        print(f"[-] CANH BAO: Khong tim thay {VMOUSE_PATH}.")

    # 4. Ma hoa ms_input_core.dll -> ms_input_core.bin
    if os.path.exists(MS_INPUT_PATH):
        xor_file(MS_INPUT_PATH, MS_INPUT_OUTPUT, key)
        print(f"[+] Da ma hoa va luu Logitech BIN tai: {MS_INPUT_OUTPUT}")
    else:
        print(f"[-] CANH BAO: Khong tim thay {MS_INPUT_PATH}.")

    # 5. Cap nhat key vao header C++ de App giai ma duoc
    with open(KEY_HEADER_PATH, "w") as f:
        f.write("#pragma once\n")
        f.write("#include <string>\n\n")
        f.write(f"namespace mapper::secret {{\n")
        f.write(f"    inline const std::string BUILD_KEY = \"{key}\";\n")
        f.write(f"    inline uint32_t SESSION_TOKEN = 0;\n")
        f.write(f"}}\n")
    print(f"[+] Da cap nhat Secret Key vao C++ Header: {KEY_HEADER_PATH}")

    print("\n[OK] QUY TRINH DONG GOI HOAN TAT! HAY BUILD LAI SECURITYHEALTHSERVICE TRONG VS.")

if __name__ == "__main__":
    pack()

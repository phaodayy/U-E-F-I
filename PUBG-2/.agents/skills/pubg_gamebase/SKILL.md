# Skill: PUBG Gamebase Analysis

Dùng để tra cứu mô hình vận hành và phân cấp bộ nhớ của game PUBG dựa trên tài liệu `gamebaseRael.txt`.

## 🏗 Cấu trúc phân cấp bộ nhớ (Memory Hierarchy)

Mô hình vận hành của game dựa trên Unreal Engine 4, được phân cấp như sau:

1.  **ImageBase (TslGame.exe)**
    *   `XenuineDecrypt`: Hàm giải mã pointer nhạy cảm (XeDecrypt).
    *   `GNames`: Danh sách tên các string trong game.
    *   `UWorld` (XeDecrypt): Gốc của toàn bộ dữ liệu game.

2.  **UWorld -> GameInstance -> LocalPlayer -> PlayerController**
    *   `PlayerCameraManager`: Quản lý Camera, FOV, Rotation.
    *   `AcknowledgedPawn` (XeDecrypt): Nhan vật của người chơi (Pawn).

3.  **Pawn (Character)**
    *   `RootComponent` (XeDecrypt): Vị trí (Position) và Vận tốc (Velocity).
    *   `Mesh`: Khung xương và hoạt ảnh (Animation).
        *   `BoneArray`: Mảng tọa độ các khớp xương (Head=6, Neck=5, v.v.).
        *   `AnimScriptInstance`: Chứa dữ liệu Recoil, Lean, Control Rotation.
    *   `WeaponProcessor`: Quản lý vũ khí.
        *   `EquippedWeapons`: Mảng vũ khí đang mang (3 slot).
        *   `CurrentWeaponIndex`: Vũ khí đang cầm trên tay.
    *   `VehicleRiderComponent`: Thông tin khi đang ngồi trên xe.

4.  **UWorld -> CurrentLevel -> ActorList**
    *   Chứa toàn bộ các thực thể xung quanh (Players, Items, Vehicles).
    *   Mỗi Actor có `ObjID` cần giải mã qua `DecryptCIndex` để lấy tên từ `GNames`.

## 📂 Quy tắc giải mã (Decryption Rules)

*   **XeDecrypt**: Cần dùng cho `UWorld`, `GameInstance`, `LocalPlayer`, `PlayerController`, `AcknowledgedPawn`, `RootComponent`, `CurrentLevel`, `Actors`.
*   **DecryptCIndex**: Chỉ dùng cho `ObjID` (32-bit).
*   **No Decrypt**: Một số pointer như `Mesh`, `PlayerState`, `BoneArray` là plain pointer (uintptr_t).

## 🛠 Cách sử dụng thông tin này
Khi update logic Macro hoặc ESP, luôn đối chiếu với sơ đồ này để biết pointer nào cần giải mã và nằm ở đâu trong cây phân cấp.

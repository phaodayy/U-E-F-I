# �️ Scanner Signature Fix Plan
# 🛡️ Scanner Signature Fix Plan

Danh sách các biến trong `main.cpp` đang có Signature chưa chuẩn (trả về giá trị rác, Opcode hoặc sai lệch hoàn toàn). Cần tìm mỏ neo mới cho từng biến này.

## 1. 🔑 Hệ Thống Giải Mã (Decryption)
- [o] **DecryptNameIndexRor**: Hiện tại bắt nhầm `0x17` (Ror gốc thường là `0x1`).

## 2. 👤 Thông Tin Người Chơi (Player Info)
- [o] **PlayerName**: Hiện `0x420` (Chuẩn).
- [o] **SurvivalLevel**: Hiện `0xCCC` (Chuẩn).
- [o] **SquadMemberIndex**: Hiện `0xA1C` (Chuẩn).
- [o] **DamageDealtOnEnemy**: Hiện `0x804` (Chuẩn).
- [x] **AccountId**: Hiện `NULL` (Mỏ neo không khớp).
- [x] **PlayerStatistics**: Hiện `0x7E8` (Cần Verify so với 0xA10).
- [ ] **ping**: Hiện `0x238` (Cần Verify so với 0x3F8).

## 3. ⚔️ Combat & Trạng Thái (Combat/CP)
- [o] **VerticalRecovery**: Hiện `0x10D8` (Chuẩn).
- [x] **LeanLeftAlpha_CP**: Hiện `0xBC0` (Sai - Master 0x694).
- [x] **LeanRightAlpha_CP**: Hiện `0x404` (Sai - Master 0x698).
- [x] **ElapsedCookingTime**: Hiện `0x8F887` (Sai).
- [x] **bIsScoping_CP**: Hiện `0x75000008` (Sai).
- [x] **bIsReloading_CP**: Hiện `0x75000007` (Sai).
- [ ] **LeanRightAlpha_CP**: Hiện `0x404` (Master 0x698).
- [ ] **ControlRotation_CP**: Hiện `0xBC8` (Master 0x64C).

## 4. 🚗 Phương Tiện & Engine (Vehicles/Misc)
- [ ] **VehicleRiderComponent**: Hiện `0x1928BD8B` (Sai).
- [ ] **VehicleFuel**: Hiện `0x81100FF3` (Sai).
- [ ] **VehicleHealthMax**: Hiện `0x48000002` (Sai).
- [ ] **ReplicatedMovement**: Hiện `0x588A` (Master 0xD0).
- [ ] **Durability**: Hiện `0x8108010` (Sai).
- [ ] **TimeSeconds**: Hiện `0x9C` (Master 0x810).

- [ ] **SurvivalTier**
- [ ] **BallisticCurve**


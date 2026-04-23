# �️ Scanner Signature Fix Plan

Danh sách các biến trong `main.cpp` đang có Signature chưa chuẩn (trả về giá trị rác, Opcode hoặc sai lệch hoàn toàn). Cần tìm mỏ neo mới cho từng biến này.

## 1. 🔑 Hệ Thống Giải Mã (Decryption)
- [o] **DecryptNameIndexRor**: Hiện tại bắt nhầm `0x17` (Ror gốc thường là `0x1`).

## 2. 👤 Thông Tin Người Chơi (Player Info) - Đang dính Opcode
- [o] **PlayerName**: Hiện `0x48000004` (Sai).
- [o] **SurvivalLevel**: Hiện `0x8B480032` (Sai).
- [o] **SquadMemberIndex**: Hiện `0x3C8858B` (Sai).
- [x] **PlayerStatistics**: Hiện `0x3F0988B` (Sai).
- [x] **AccountId**: Hiện `0x32886` (Cần Verify).
- [x] **MatchId**: Hiện `0x49897` (Sai).
- [ ] **ping**: Hiện `0x238` (Sai lệch so với Master 0x3F8).
- [o] **DamageDealtOnEnemy**: Hiện `0x3F0988B` (Sai).

## 3. ⚔️ Combat & Trạng Thái (Combat/CP)
- [ ] **ElapsedCookingTime**: Hiện `0x8F887` (Sai).
- [ ] **bIsScoping_CP**: Hiện `0x75000008` (Bắt nhầm hằng số).
- [ ] **bIsReloading_CP**: Hiện `0x75000007` (Bắt nhầm hằng số).
- [ ] **LeanLeftAlpha_CP**: Hiện `0xBC0` (Master 0x694).
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


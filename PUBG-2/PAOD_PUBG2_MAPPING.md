# Mapping: `PAOD-PUBG-DMA-Source-Code-main` -> `hyper-reV/PUBG-2`

Muc dich cua file nay la tao ban do doi chieu de sau nay port chuc nang nhanh hon, khong phai doan lai toan bo codebase moi lan.

Nguon tham chieu:
- PAOD: `D:\DMA\PAOD-PUBG-DMA-Source-Code-main`
- Dich hien tai: `D:\HyperVesion\UEFI\hyper-reV\PUBG-2`

## Quy uoc doc mapping

- `PAOD` = repo tham chieu DMA cu.
- `PUBG-2` = repo dang phat trien hien tai.
- `Active path` = luong code dang chi phoi ban build/overlay hien tai.
- `Legacy path` = code cu van con trong repo, huu ich de tham chieu logic, nhung khong phai luong chinh dang chay.

## Tong quan kien truc

PAOD la kien truc `VMM/DMA-first`:
- attach qua `vmm.dll` / `leechcore.dll`
- su dung SDK abstraction kha day
- nhieu logic nam trong `Core/Visuals.cpp` va `Core/SDK/*`

PUBG-2 hien tai la kien truc `hyper-first`:
- active read/write di qua `sdk/memory.hpp` + `sdk/hyper_process.cpp` + `sdk/hypercall_bridge.cpp`
- game scan/cache player nam chu yeu trong `sdk/context.cpp`
- render/menu nam trong `overlay/overlay_menu.*`
- trong repo van con mot nhanh DMA cu o `sdk/Players.h`, `sdk/Common/*`, `sdk/DMALibrary/*`

## Mapping cap cao

| Chuc nang | PAOD | PUBG-2 | Ghi chu |
|---|---|---|---|
| Entry / bootstrap | [Main.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Main.cpp), [Core/Core.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/Core.cpp) | [pubg/main.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/main.cpp) | Day la noi khoi dong, license, doi process, khoi tao overlay |
| Memory backend | [Core/DMA/DMAHandler.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/DMA/DMAHandler.h), [Core/DMA/DMAHandler.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/DMA/DMAHandler.cpp) | [pubg/sdk/memory.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/memory.hpp), [pubg/sdk/hyper_process.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/hyper_process.cpp), [pubg/sdk/hypercall_bridge.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/hypercall_bridge.cpp) | Khong port API 1:1. Chi port logic, bo phan read/write phai viet theo hyper |
| Offsets / patterns | [Core/PatternLoader.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/PatternLoader.h), [Core/PatternLoader.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/PatternLoader.cpp) | [.shared/pubg_config.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/.shared/pubg_config.hpp), [pubg/sdk/offsets.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/offsets.hpp), [Offset_pubg.txt](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/Offset_pubg.txt) | PUBG-2 dung offset static/local, khong co pattern host-loader giong PAOD |
| Decrypt / encrypted UE access | [Core/SDK/UEncrypt.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/UEncrypt.h), [Core/SDK/UEncrypt.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/UEncrypt.cpp) | [pubg/sdk/pubg_decrypt.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/pubg_decrypt.hpp), [pubg/sdk/Decrypt.*](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Decrypt.h) | Dung de doi chieu cach decrypt UWorld, names, health, object references |
| GNames / object naming | [Core/SDK/UnrealNames.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/UnrealNames.h), [Core/SDK/ObjectsStore.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/ObjectsStore.cpp) | [pubg/sdk/fname.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/fname.hpp), [pubg/sdk/fname_utils.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/fname_utils.cpp), [pubg/sdk/GNames.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/GNames.h) | Neu can port logic phan loai actor/item/weapon, bat dau o day |
| SDK abstraction / UObject wrappers | [Core/SDK/SDK.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/SDK.h), [Core/SDK/CoreUObject/Class.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/CoreUObject/Class.h), [Core/SDK/ObjectManager.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/ObjectManager.h) | [pubg/sdk/Common/Data.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Common/Data.h), [pubg/sdk/Common/Offset.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Common/Offset.h), [pubg/sdk/Actors.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Actors.h) | PAOD truu tuong hon nhieu; PUBG-2 thuc dung hon, it wrapper hon |
| Player scan / cache | PAOD chu yeu trong [Core/Visuals.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/Visuals.cpp) va `Core/SDK/*` | Active: [pubg/sdk/context.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/context.cpp). Legacy: [pubg/sdk/Players.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Players.h) | Khi port tinh nang player overlay, uu tien `context.cpp`; khi can logic sau hon, tham khao `Players.h` |
| Player visuals / ESP | [Core/Visuals.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/Visuals.h), [Core/Visuals.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/Visuals.cpp) | [pubg/overlay/overlay_menu.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/overlay/overlay_menu.hpp), [pubg/overlay/overlay_menu.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/overlay/overlay_menu.cpp) | PAOD gop rat nhieu vao `Visuals.cpp`; PUBG-2 tach scan va render ro hon |
| Config / menu state | [Core/Config.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/Config.h), [Core/Config.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/Config.cpp) | Active UI state: [pubg/overlay/overlay_menu.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/overlay/overlay_menu.hpp). Legacy config: [pubg/sdk/Common/Config.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Common/Config.h) | Neu port menu/value, can quyet dinh no thuoc active overlay hay legacy sdk |
| Item / vehicle | PAOD: `Core/ItemManager.*`, `Core/VehicleManager.*`, `Core/SkinWeaponManager.*` | [pubg/sdk/Items.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Items.h), [pubg/sdk/Vehicles.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Vehicles.h), [pubg/sdk/Projects.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Projects.h) | Chuc nang da duoc tach theo loai trong PUBG-2 |
| Visibility | PAOD nam trong pipeline player/visuals | Active: [pubg/sdk/context.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/context.cpp). Legacy helper: [pubg/sdk/VisibleCheck.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/VisibleCheck.h) | Hien da chot active path la `local Eyes` cho overlay |
| WorldToScreen / math | PAOD trong SDK/visuals math | [pubg/sdk/context.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/context.cpp), [pubg/sdk/Common/VectorHelper.*](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Common/VectorHelper.cpp), [pubg/sdk/math.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/math.hpp) | `context.cpp` la active W2S cua overlay moi |
| Aimbot / recoil / input | PAOD chu yeu trong `Visuals.cpp` + cac manager lien quan | [pubg/sdk/AimBot.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/AimBot.h), [pubg/sdk/autoRecoil.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/autoRecoil.h), [pubg/sdk/MouseDispatcher.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/MouseDispatcher.h), [pubg/sdk/vmouse_client.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/vmouse_client.hpp) | Input backend cua PUBG-2 khac PAOD, khong port nguyen xi |
| Physics / line trace / scene | PAOD dua vao PhysX va game scene | [pubg/sdk/PhysXManager.*](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/PhysXManager.cpp), [pubg/sdk/LineTrace.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/LineTrace.h), [pubg/sdk/VisibleCheck.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/VisibleCheck.h) | Phan nay co the tham khao PAOD khi can vis/raycast nang |
| Radar / web / net | PAOD co radar va host interaction trong `Core.cpp`, `Visuals.cpp` | [pubg/sdk/Radar.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Radar.h), [pubg/sdk/WebRadar.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/WebRadar.h), [pubg/sdk/WebSocketClient.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/WebSocketClient.h) | Nen port theo y tuong, khong copy backend |
| Driver / protection / loader | [Core/DriverControl.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/DriverControl.cpp), `dll/*` | [driver/](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/driver), [loader/](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/loader), [standalone_mapper/](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/standalone_mapper), [pubg/mapper/mapper.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/mapper/mapper.cpp) | Day la khu vuc rat de lech moi truong; chi doi chieu y tuong |

## Active path cua `PUBG-2`

Neu muc tieu la sua ban build dang dung trong `hyper-reV/PUBG-2`, hay coi day la luong chinh:

1. [pubg/main.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/main.cpp)
   - license
   - wait PID
   - connect hyper
   - init overlay / game loop

2. [pubg/sdk/memory.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/memory.hpp)
   - active memory transport
   - `RefreshProcessContext()`
   - `ReadMemory()` / `WriteMemory()`

3. [pubg/sdk/hyper_process.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/hyper_process.cpp)
   - query CR3
   - resolve process context
   - kernel/process support cho backend hyper

4. [pubg/sdk/context.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/context.cpp)
   - update UWorld/local player/camera
   - actor scan
   - player cache
   - active visibility
   - active skeleton/gender mapping
   - active W2S cua overlay

5. [pubg/overlay/overlay_menu.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/overlay/overlay_menu.cpp)
   - ve ESP
   - menu ImGui
   - aimbot glue voi player cache

## Legacy path huu ich trong `PUBG-2`

Nhung file duoi day khong phai luong overlay hyper hien tai, nhung rat co gia tri de muon logic:

- [pubg/sdk/Players.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Players.h)
  - pipeline player day hon
  - scatter read
  - visible logic kieu local render time
  - gender-aware bones
  - rank / team / cache logic

- [pubg/sdk/Common/Data.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Common/Data.h)
  - struct data lon, dung de hieu schema cu

- [pubg/sdk/Common/VectorHelper.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/Common/VectorHelper.cpp)
  - W2S / math cu

- [pubg/sdk/VisibleCheck.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/VisibleCheck.h)
  - helper vis theo `enemyEyes + 0.05 >= localEyes`

## Porting rules tu PAOD sang `PUBG-2`

### 1. Port logic, khong port backend

PAOD doc bo nho qua `DMAHandler`/`VMMDLL`.
PUBG-2 active doc bo nho qua `PubgMemory::ReadMemory()` tren hyper.

Vi vay:
- Cho phep muon logic scan, cach loc actor, cach tinh skeleton, cach layout UI
- Khong copy nguyen xi read/scatter/dma API

### 2. Neu tinh nang lien quan den player ESP

Diem vao tot nhat:
- PAOD: `Core/Visuals.cpp`
- PUBG-2 dich: `pubg/sdk/context.cpp` + `pubg/overlay/overlay_menu.cpp`

Mau tu duy:
- scan/cache o `context.cpp`
- render o `overlay_menu.cpp`
- neu can data sau hon, tham khao `Players.h`

### 3. Neu tinh nang lien quan den offset/decrypt

So sanh:
- PAOD: `PatternLoader.*`, `UEncrypt.*`, `UnrealNames.*`
- PUBG-2: `.shared/pubg_config.hpp`, `sdk/offsets.hpp`, `sdk/pubg_decrypt.hpp`, `sdk/fname*`

Dung PAOD de hieu y nghia offset/decrypt.
Dung PUBG-2 de dat gia tri/implement cach doc that su.

### 4. Neu tinh nang lien quan den visibility

PAOD thuong tron trong player/visual pipeline.
PUBG-2 hien co 2 noi tham chieu:
- Active overlay vis: [pubg/sdk/context.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/context.cpp)
- Legacy/helper vis: [pubg/sdk/VisibleCheck.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/VisibleCheck.h)

Quy tac:
- Muon thay doi mau sac ESP/target lock theo vis: sua `context.cpp`
- Muon nghien cuu cong thuc vis khac: doi chieu them `VisibleCheck.h` va `Players.h`

### 5. Neu tinh nang lien quan den aimbot

PAOD tham chieu:
- `Core/Visuals.cpp`
- `Visuals.h`

PUBG-2 dich:
- [pubg/sdk/AimBot.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/AimBot.h)
- [pubg/overlay/overlay_menu.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/overlay/overlay_menu.cpp)
- [pubg/sdk/MouseDispatcher.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/MouseDispatcher.h)
- [pubg/sdk/vmouse_client.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/vmouse_client.hpp)

Tach van de thanh 3 lop:
- target selection
- world/screen math
- input output

### 6. Neu tinh nang lien quan den item/vehicle/radar

PAOD:
- `ItemManager.*`
- `VehicleManager.*`
- radar/network trong `Core.cpp` va `Visuals.cpp`

PUBG-2:
- `sdk/Items.h`
- `sdk/Vehicles.h`
- `sdk/Radar.h`
- `sdk/WebRadar.h`

Day la khu vuc mapping kha thang, de port hon phan player core.

## Mapping theo nhu cau thuong gap

### Muon port player box / skeleton / name / weapon ESP

- PAOD doc: [Core/Visuals.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/Visuals.cpp)
- PUBG-2 sua: [pubg/sdk/context.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/context.cpp), [pubg/overlay/overlay_menu.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/overlay/overlay_menu.cpp)

### Muon port visible check / line trace / local Eyes

- PAOD doc: `Core/Visuals.cpp` + SDK player pipeline
- PUBG-2 sua: [pubg/sdk/context.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/context.cpp), [pubg/sdk/VisibleCheck.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/VisibleCheck.h), [pubg/sdk/LineTrace.h](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/LineTrace.h)

### Muon port actor/object filtering

- PAOD doc: [Core/SDK/ObjectManager.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/ObjectManager.h), `Core/SDK/*`
- PUBG-2 sua: [pubg/sdk/context.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/context.cpp), [pubg/sdk/fname_utils.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/fname_utils.cpp)

### Muon port decryption / encrypted object access

- PAOD doc: [Core/SDK/UEncrypt.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/UEncrypt.cpp), [Core/SDK/CoreUObject/Class.h](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/SDK/CoreUObject/Class.h)
- PUBG-2 sua: [pubg/sdk/pubg_decrypt.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/pubg_decrypt.hpp), [.shared/pubg_config.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/.shared/pubg_config.hpp)

### Muon port process attach / PID / module base logic

- PAOD doc: [Core/DMA/DMAHandler.cpp](/d:/DMA/PAOD-PUBG-DMA-Source-Code-main/Core/DMA/DMAHandler.cpp)
- PUBG-2 sua: [pubg/main.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/main.cpp), [pubg/sdk/hyper_process.cpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/hyper_process.cpp), [pubg/sdk/memory.hpp](/d:/HyperVesion/UEFI/hyper-reV/PUBG-2/pubg/sdk/memory.hpp)

## Canh bao khi doi chieu

### Khong nen copy nguyen xi tu PAOD

Nhung phan duoi day phai xem nhu reference y tuong, khong copy thang:
- `DMAHandler`
- `DriverControl`
- network host/pattern fetch
- VMProtect / loader / anti-debug glue
- direct SDK wrappers phu thuoc offset layout cu

### Trong `PUBG-2` co 2 he thong song song

1. He thong active:
- `main.cpp`
- `sdk/memory.hpp`
- `sdk/hyper_process.*`
- `sdk/context.*`
- `overlay/overlay_menu.*`

2. He thong legacy/tham chieu:
- `sdk/Players.h`
- `sdk/Common/*`
- `sdk/DMALibrary/*`

Neu khong xac dinh ro dang sua he thong nao, rat de port nham file.

## Loi khuyen thao tac tu nay ve sau

Khi ban muon "lay tinh nang tu PAOD", hay mo ta theo mau nay:

1. Tinh nang nao?
2. Dang nhin o file nao ben PAOD?
3. Muon dua vao `active overlay path` hay `legacy sdk path` cua `PUBG-2`?

Neu khong noi ro, mac dinh nen dua vao `active overlay path`.

## Ket luan nhanh

Neu can muon logic tu PAOD cho ban build `hyper-reV/PUBG-2` dang dung hien tai, thu tu uu tien nen la:

1. Doc PAOD de lay y tuong / quy trinh.
2. Tim diem dat tuong ung trong `PUBG-2` active path.
3. Viet lai theo backend hyper.
4. Chi quay sang `Players.h`/`Common/*` khi active path chua co du context.


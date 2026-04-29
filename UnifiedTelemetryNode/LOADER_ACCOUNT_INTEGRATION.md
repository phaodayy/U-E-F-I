# Hướng Dẫn Tích Hợp Account Licensing

Tài liệu này mô tả flow licensing theo tài khoản cho client bên ngoài. Client đăng nhập bằng tài khoản, nhận JWT có `session_id`, dùng key để cộng thời gian vào tài khoản, rồi gửi heartbeat để duy trì phiên.

Model mới không lưu HWID theo key. HWID chỉ dùng ở cấp tài khoản/session để đảm bảo một tài khoản chỉ active trên một máy tại một thời điểm.

## Tổng Quan

Base URL production:

```text
https://licensing-backend.donghiem114.workers.dev
```

Postman collection:

```text
GZ-Loader-Account-API.postman_collection.json
```

Endpoint chính:

| Mục đích | Endpoint |
| --- | --- |
| Tạo account user | `POST /loader/register` |
| Đăng nhập và tạo session mới | `POST /loader/login` |
| Lấy account/session hiện tại | `GET /loader/me` |
| Redeem key để cộng giờ | `POST /loader/keys/activate` |
| Kiểm tra phiên/license định kỳ | `POST /loader/heartbeat` |
| Lấy config account | `GET /loader/config` |
| Lưu config account | `PUT /loader/config` |
| Import config từ mã chia sẻ | `POST /loader/config/import` |

## Chuẩn Bị Server

Backend cần các binding/secret:

```text
DB              D1 database binding
JWT_SECRET      secret để ký JWT đăng nhập
RSA_PRIVATE_KEY private key để ký response activate/heartbeat
```

Chạy migration:

```text
backend/migrations/0007_loader_accounts_configs.sql
```

Migration tạo các bảng riêng cho account chơi game, không dùng chung bảng `users` của admin/seller:

- `loader_accounts`
- `loader_key_redemptions`
- `loader_configs`
- `loader_config_imports`
- `loader_account_events`

Migration cũng tạo account admin mặc định cho hệ account:

```text
username: loader_admin
password: admin123
role: ADMIN
```

Đổi mật khẩu admin mặc định ngay sau khi setup.

## Model License

Account chỉ có hai role:

- `ADMIN`
- `USER`

Khi người dùng tự đăng ký, role mặc định là `USER`.

Key không còn là license đang chạy. Key chỉ là voucher cộng thời gian:

1. User đăng nhập account.
2. User nhập key.
3. Server kiểm tra key còn `NEW`.
4. Server lấy `duration_minutes` của key type.
5. Server cộng thời gian vào `loader_accounts.expires_at`.
6. Server đánh dấu key là `ACTIVATED`.
7. Server ghi lịch sử vào `loader_key_redemptions`.

Nếu account còn hạn, thời gian mới được cộng nối tiếp từ hạn hiện tại. Nếu account đã hết hạn hoặc chưa có hạn, thời gian được cộng từ thời điểm redeem.

## Chính Sách Một Máy

Mỗi lần login/register thành công, server tạo `session_id` mới và lưu:

- `loader_accounts.current_hwid`
- `loader_accounts.current_session_id`
- `loader_accounts.current_login_at`
- `loader_accounts.last_seen_at`

JWT trả về cho client cũng chứa `session_id` và `hwid`.

Khi cùng account đăng nhập ở máy khác, server ghi đè `current_hwid/current_session_id`. Token ở máy cũ vẫn còn hạn về mặt JWT, nhưng mọi request tiếp theo sẽ bị trả:

```json
{
  "status": "SESSION_REPLACED",
  "active": false
}
```

Client phải xử lý `SESSION_REPLACED` bằng cách dừng phiên hiện tại và quay về màn hình đăng nhập.

## Quy Tắc Bảo Mật Client

Client không được nhúng:

- `JWT_SECRET`
- `RSA_PRIVATE_KEY`

Client cần lưu:

- `username`
- JWT token trả về từ `/loader/login` hoặc `/loader/register`
- `session_id`
- HWID hiện tại
- public key RSA để verify chữ ký server

Client phải tạo nonce mới cho mỗi request signed:

- Độ dài: `32-128` ký tự.
- Charset: `A-Z`, `a-z`, `0-9`, `_`, `-`.
- Không dùng lại nonce. Server sẽ trả `REPLAY_DETECTED`.

Client phải verify `signature` trên đúng chuỗi `signed_payload`. Sau khi verify thành công mới parse `signed_payload` và tin các trường license bên trong.

Response signed hiện có:

- `POST /loader/keys/activate`
- `POST /loader/heartbeat`

## Flow Tích Hợp Khuyến Nghị

1. Client lấy HWID ổn định của máy.
2. Client gọi `/loader/login` bằng `username/password/hwid`.
3. Server tạo session mới. Nếu account đang online ở máy khác, máy kia sẽ bị đá ở heartbeat tiếp theo.
4. Client lưu `token` và `session_id`.
5. Client gọi `/loader/me?hwid=<HWID>` để lấy `entitlement`.
6. Nếu `entitlement.active` là `false`, yêu cầu user nhập key và gọi `/loader/keys/activate`.
7. Client verify RSA signature của response activate.
8. Trong lúc app chạy, client gửi `/loader/heartbeat` định kỳ.
9. Nếu heartbeat trả `SESSION_REPLACED`, `SESSION_HWID_MISMATCH`, `EXPIRED`, `BANNED` hoặc signature không hợp lệ, dừng phiên.

JWT hiện có hạn 24 giờ. Khi hết hạn, server trả `401`; client cần login lại.

## Header Chung

Endpoint public account:

```http
Content-Type: application/json
User-Agent: GZ-Account-Loader
```

Endpoint cần đăng nhập:

```http
Authorization: Bearer <LOADER_TOKEN>
Content-Type: application/json
User-Agent: GZ-Account-Loader
```

## Đăng Ký

```http
POST /loader/register
```

Request:

```json
{
  "username": "customer01",
  "password": "strong-password",
  "email": "customer01@example.com",
  "hwid": "HWID_HASH_16"
}
```

`email` optional. `hwid` bắt buộc vì register cũng tạo session đầu tiên.

Response:

```json
{
  "status": "REGISTERED",
  "token": "JWT_TOKEN",
  "session_id": "SESSION_UUID",
  "user": {
    "id": 10,
    "username": "customer01",
    "email": "customer01@example.com",
    "role": "USER",
    "status": "ACTIVE",
    "expires_at": null,
    "current_hwid": "HWID_HASH_16",
    "current_login_at": "2026-04-29T12:00:00.000Z",
    "last_seen_at": "2026-04-29T12:00:00.000Z",
    "language": "vi"
  },
  "config_code": "CFG-XXXXXXXXXXXX",
  "entitlement": {
    "status": "NO_ACTIVE_TIME",
    "active": false,
    "expires_at": null,
    "remaining_seconds": 0,
    "key": "",
    "product": null,
    "key_type": null
  }
}
```

## Đăng Nhập

```http
POST /loader/login
```

Request:

```json
{
  "username": "customer01",
  "password": "strong-password",
  "hwid": "HWID_HASH_16"
}
```

Response:

```json
{
  "status": "OK",
  "token": "JWT_TOKEN",
  "session_id": "SESSION_UUID",
  "user": {
    "id": 10,
    "username": "customer01",
    "role": "USER",
    "status": "ACTIVE",
    "expires_at": "2026-05-01T12:00:00.000Z",
    "current_hwid": "HWID_HASH_16",
    "current_login_at": "2026-04-29T12:00:00.000Z",
    "last_seen_at": "2026-04-29T12:00:00.000Z",
    "language": "vi"
  },
  "config_code": "CFG-XXXXXXXXXXXX",
  "entitlement": {
    "status": "ACTIVE",
    "active": true,
    "expires_at": "2026-05-01T12:00:00.000Z",
    "remaining_seconds": 172800,
    "key": "PRODUCT-XXXX-XXXX",
    "product": "Product Name",
    "key_type": "30 Days"
  }
}
```

Mỗi lần login thành công sẽ thay session cũ. Đây là cơ chế đá máy khác.

## Lấy Profile Hiện Tại

```http
GET /loader/me?hwid=HWID_HASH_16
Authorization: Bearer <LOADER_TOKEN>
```

Response giống `/loader/login` nhưng không trả token mới. Nếu token không còn là session hiện tại, server trả `SESSION_REPLACED`.

## Redeem Key Để Cộng Giờ

```http
POST /loader/keys/activate
Authorization: Bearer <LOADER_TOKEN>
```

Request:

```json
{
  "key": "PRODUCT-XXXX-XXXX",
  "hwid": "HWID_HASH_16",
  "nonce": "64_HEX_OR_URL_SAFE_RANDOM_CHARS",
  "signature_version": 2
}
```

Response thành công:

```json
{
  "status": "ACTIVATED",
  "active": true,
  "account_id": 10,
  "username": "customer01",
  "role": "USER",
  "session_id": "SESSION_UUID",
  "hwid": "HWID_HASH_16",
  "key": "PRODUCT-XXXX-XXXX",
  "product": "Product Name",
  "key_type": "30 Days",
  "expiry": "2026-05-01T12:00:00.000Z",
  "expires_at": "2026-05-01T12:00:00.000Z",
  "remaining_seconds": 172800,
  "config_code": "CFG-XXXXXXXXXXXX",
  "device_policy": "SINGLE_ACTIVE_SESSION",
  "timestamp": "2026-04-29T12:00:00.000Z",
  "issued_at": 1777464000,
  "nonce": "64_HEX_OR_URL_SAFE_RANDOM_CHARS",
  "request_id": "uuid",
  "signature_version": 2,
  "signed_payload": "{\"v\":2,\"action\":\"activate\",...}",
  "signature": "RSA_SIGNATURE_BASE64"
}
```

Client phải verify:

- `signature_version` là `2`.
- RSA signature hợp lệ với đúng chuỗi `signed_payload`.
- Trong payload: `action` là `activate`.
- Trong payload: `account_id`, `username`, `session_id`, `hwid`, `key`, `nonce` khớp request/session.
- `device_policy` là `SINGLE_ACTIVE_SESSION`.
- `issued_at` còn mới, khuyến nghị tối đa `300` giây.
- `active` là `true` và `status` là `ACTIVATED`.

Key chỉ dùng được một lần. Nếu key đã được redeem, server trả `KEY_ALREADY_USED`.

## Heartbeat

```http
POST /loader/heartbeat
Authorization: Bearer <LOADER_TOKEN>
```

Request:

```json
{
  "hwid": "HWID_HASH_16",
  "nonce": "64_HEX_OR_URL_SAFE_RANDOM_CHARS",
  "signature_version": 2
}
```

`key` không còn bắt buộc vì hạn nằm trên account. Nếu client gửi `key`, server chỉ echo lại trong response khi phù hợp.

Response thành công:

```json
{
  "status": "OK",
  "active": true,
  "account_id": 10,
  "username": "customer01",
  "role": "USER",
  "session_id": "SESSION_UUID",
  "hwid": "HWID_HASH_16",
  "key": "PRODUCT-XXXX-XXXX",
  "product": "Product Name",
  "key_type": "30 Days",
  "expiry": "2026-05-01T12:00:00.000Z",
  "expires_at": "2026-05-01T12:00:00.000Z",
  "remaining_seconds": 172500,
  "config_code": "CFG-XXXXXXXXXXXX",
  "device_policy": "SINGLE_ACTIVE_SESSION",
  "timestamp": "2026-04-29T12:05:00.000Z",
  "issued_at": 1777464300,
  "nonce": "64_HEX_OR_URL_SAFE_RANDOM_CHARS",
  "request_id": "uuid",
  "signature_version": 2,
  "signed_payload": "{\"v\":2,\"action\":\"heartbeat\",...}",
  "signature": "RSA_SIGNATURE_BASE64"
}
```

Client verify giống activate, nhưng `action` phải là `heartbeat` và `status` phải là `OK`.

Tần suất heartbeat nên do sản phẩm quyết định. Gợi ý thực tế là 30-60 giây một lần, và gửi lại ngay khi app resume sau sleep/network reconnect.

## Config Tài Khoản

Mỗi account có một config JSON và một mã chia sẻ `config_code`.

### Lấy Config

```http
GET /loader/config
Authorization: Bearer <LOADER_TOKEN>
```

### Lưu Config

```http
PUT /loader/config
Authorization: Bearer <LOADER_TOKEN>
```

Request:

```json
{
  "config": {
    "theme": "dark",
    "language": "vi"
  }
}
```

Giới hạn config là `128 KB`. Nếu vượt quá, server trả `CONFIG_TOO_LARGE`.

### Import Config

```http
POST /loader/config/import
Authorization: Bearer <LOADER_TOKEN>
```

Request:

```json
{
  "code": "CFG-XXXXXXXXXXXX"
}
```

## Status Và Lỗi Phổ Biến

| Status | Ý nghĩa | Client nên làm gì |
| --- | --- | --- |
| `REGISTERED` | Tạo account thành công | Lưu token/session |
| `OK` | Login/heartbeat thành công | Tiếp tục phiên |
| `ACTIVATED` | Key đã cộng giờ vào account | Verify chữ ký, cập nhật expiry |
| `NO_ACTIVE_TIME` | Account chưa có thời gian chơi | Yêu cầu nhập key |
| `EXPIRED` | Account đã hết hạn | Yêu cầu gia hạn/key mới |
| `INVALID_CREDENTIALS` | Sai username/password | Yêu cầu nhập lại |
| `ACCOUNT_EXISTS` | Username/email đã tồn tại | Yêu cầu đổi username/email |
| `ACCOUNT_LOCKED` | Account bị khóa | Dừng phiên, liên hệ admin |
| `SESSION_REPLACED` | Account đã login ở máy/session khác | Logout ngay ở máy hiện tại |
| `SESSION_HWID_MISMATCH` | HWID request không khớp session | Logout và login lại |
| `INVALID_KEY` | Key không tồn tại | Yêu cầu nhập key khác |
| `KEY_ALREADY_USED` | Key đã được redeem | Không cho dùng key này |
| `BANNED` | HWID bị blacklist | Dừng phiên |
| `PRODUCT_LOCKED` | Product/key type bị tắt | Dừng phiên |
| `REPLAY_DETECTED` | Nonce đã dùng | Tạo nonce mới, kiểm tra lỗi client |
| `INVALID_NONCE` | Nonce sai format | Sửa generator nonce |
| `CONFIG_NOT_FOUND` | Mã config import không tồn tại | Yêu cầu nhập mã khác |
| `CONFIG_TOO_LARGE` | Config vượt 128 KB | Giảm kích thước config |
| `RATE_LIMITED` | Quá nhiều lần login/register lỗi | Chờ rồi thử lại |

## Pseudocode Client

```text
hwid = collectStableHwid()

login_response = POST /loader/login(username, password, hwid)
token = login_response.token
session_id = login_response.session_id

profile = GET /loader/me?hwid=hwid with token

if profile.entitlement.active is false:
    key = askUserForKey()
    nonce = randomNonce()
    response = POST /loader/keys/activate(token, key, hwid, nonce)
    verifySignedResponse(response, "activate", session_id, key, hwid, nonce)

while app is running:
    wait heartbeat interval
    nonce = randomNonce()
    response = POST /loader/heartbeat(token, hwid, nonce)

    if response.status == "SESSION_REPLACED":
        logoutImmediately()

    verifySignedResponse(response, "heartbeat", session_id, "", hwid, nonce)
```

## Checklist Cho Bên Tích Hợp

- Dùng HTTPS cho mọi request.
- Không nhúng `JWT_SECRET` hoặc `RSA_PRIVATE_KEY` vào client.
- Pin public key RSA trong client để verify response signed.
- Tạo nonce bằng cryptographic random.
- Không dùng lại nonce.
- Không tin `status`, `expiry`, `remaining_seconds`, `product`, `key_type` nếu chưa verify `signed_payload`.
- Xử lý `401` bằng login lại.
- Xử lý `409 SESSION_REPLACED` bằng logout ngay vì account đã login ở máy khác.
- Xử lý `403` bằng dừng phiên license hoặc yêu cầu người dùng nhập key mới.
- Không log password, token, private key hoặc response signed đầy đủ ở production.

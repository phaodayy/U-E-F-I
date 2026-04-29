# GZ Licensing Account API

Base URL:

```text
https://licensing-backend.donghiem114.workers.dev
```

This is the current integration surface for external clients. New clients must use `/loader/*`.

## Current Model

Accounts are stored separately from admin/seller dashboard accounts:

```text
Dashboard accounts: users
Client accounts:    loader_accounts
```

Client account roles:

- `ADMIN`
- `USER`

Self-registering accounts are created as `USER`.

Keys are one-time time vouchers. A key does not own HWID, active session, or runtime expiry. Redeeming a key adds its key type `duration_minutes` to `loader_accounts.expires_at`, marks the key as `ACTIVATED`, and writes a row to `loader_key_redemptions`.

## One-Machine Session Policy

Every successful `/loader/login` or `/loader/register` creates a new `session_id` and stores the current HWID on the account:

- `loader_accounts.current_hwid`
- `loader_accounts.current_session_id`
- `loader_accounts.current_login_at`
- `loader_accounts.last_seen_at`

The JWT returned to the client includes the same `session_id` and `hwid`.

When the same account logs in from another machine or creates a new session, older tokens are rejected by protected loader endpoints:

```json
{
  "status": "SESSION_REPLACED",
  "active": false
}
```

The client must immediately stop the current session and return to login when it receives `SESSION_REPLACED`.

## Headers

Register/login:

```http
Content-Type: application/json
User-Agent: GZ-Account-Loader
```

Authenticated endpoints:

```http
Authorization: Bearer <LOADER_TOKEN>
Content-Type: application/json
User-Agent: GZ-Account-Loader
```

Do not embed these secrets in any client:

- `JWT_SECRET`
- `RSA_PRIVATE_KEY`

## Endpoints

| Purpose | Endpoint |
| --- | --- |
| Register user account | `POST /loader/register` |
| Login and replace previous session | `POST /loader/login` |
| Get current account/session | `GET /loader/me` |
| Redeem key and add account time | `POST /loader/keys/activate` |
| Validate current account/session | `POST /loader/heartbeat` |
| Get account config | `GET /loader/config` |
| Save account config | `PUT /loader/config` |
| Import config by share code | `POST /loader/config/import` |

## Register

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

`email` is optional. `hwid` is required because registration also creates the first active session.

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

## Login

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

## Current Account

```http
GET /loader/me?hwid=HWID_HASH_16
Authorization: Bearer <LOADER_TOKEN>
```

Returns the current account and entitlement. If the token is no longer the account's current session, the server returns `SESSION_REPLACED`.

## Signed Responses

`POST /loader/keys/activate` and `POST /loader/heartbeat` return signed responses.

Every signed request must include a fresh random nonce:

- Length: `32-128` characters.
- Charset: `A-Z`, `a-z`, `0-9`, `_`, `-`.
- Reuse is rejected with `REPLAY_DETECTED`.

Every signed response includes:

```json
{
  "signature_version": 2,
  "signed_payload": "{\"v\":2,\"action\":\"heartbeat\",...}",
  "signature": "RSA_SIGNATURE_BASE64"
}
```

Client verification requirements:

- Verify `signature` over the exact `signed_payload` string with the pinned RSA public key.
- Parse `signed_payload` only after signature verification succeeds.
- Require `v = 2`.
- Require `action` to match the endpoint: `activate` or `heartbeat`.
- Require `session_id`, `hwid`, and `nonce` to match the current client session/request.
- Require `device_policy = "SINGLE_ACTIVE_SESSION"`.
- Require `issued_at` to be recent. Recommended max age: `300` seconds.
- Trust `status`, `active`, `expiry`, `remaining_seconds`, `product`, and `key_type` only from the verified signed payload.

## Redeem Key

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

Success response:

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

Redeem behavior:

- Only keys with status `NEW` can be redeemed.
- Redeeming adds the key type duration to the account expiry.
- If the account still has time, the new time is appended to the existing expiry.
- If the account is expired or has no time, the new time starts from the redeem time.
- The key is then marked `ACTIVATED`.
- A used key returns `KEY_ALREADY_USED`.

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

Response:

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

Recommended heartbeat interval: `30-60` seconds, plus one immediate heartbeat after resume/reconnect.

## Config

Each account has one JSON config and one share code.

Get config:

```http
GET /loader/config
Authorization: Bearer <LOADER_TOKEN>
```

Save config:

```http
PUT /loader/config
Authorization: Bearer <LOADER_TOKEN>
```

```json
{
  "config": {
    "theme": "dark",
    "language": "vi"
  }
}
```

Import another account's config:

```http
POST /loader/config/import
Authorization: Bearer <LOADER_TOKEN>
```

```json
{
  "code": "CFG-XXXXXXXXXXXX"
}
```

Config payloads are limited to `128 KB`.

## Status Codes

| status | Meaning |
| --- | --- |
| `REGISTERED` | Account was created |
| `OK` | Login or heartbeat is valid |
| `ACTIVATED` | Key redeemed and account time extended |
| `NO_ACTIVE_TIME` | Account has no active time |
| `EXPIRED` | Account time has expired |
| `INVALID_CREDENTIALS` | Login failed |
| `ACCOUNT_EXISTS` | Username or email already exists |
| `ACCOUNT_LOCKED` | Account is disabled |
| `SESSION_REPLACED` | Account logged in from another session/device |
| `SESSION_HWID_MISMATCH` | Request HWID does not match active session |
| `INVALID_KEY` | Key does not exist |
| `KEY_ALREADY_USED` | Key was already redeemed |
| `PRODUCT_LOCKED` | Product or key type is disabled |
| `BANNED` | HWID is blacklisted |
| `INVALID_NONCE` | Nonce is missing or malformed |
| `REPLAY_DETECTED` | Nonce was already used |
| `CONFIG_NOT_FOUND` | Shared config code does not exist |
| `CONFIG_TOO_LARGE` | Config JSON is larger than 128 KB |
| `RATE_LIMITED` | Too many login/register failures |

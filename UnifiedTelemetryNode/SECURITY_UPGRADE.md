# Account Licensing Security Model

This document describes the current security model for account-based licensing.

## Goals

- Do not expose server secrets in the client.
- Keep game/client accounts separate from dashboard admin/seller accounts.
- Make keys one-time vouchers that add time to an account.
- Allow only one active session per account.
- Let a login on a new machine invalidate the old machine.
- Sign license-critical responses so clients can detect fake-server JSON and replayed responses.

## Current Account Tables

Dashboard accounts remain in:

```text
users
```

Client accounts use:

```text
loader_accounts
loader_key_redemptions
loader_configs
loader_config_imports
loader_account_events
```

`loader_accounts.role` is either `ADMIN` or `USER`. Self-registration always creates `USER`.

## Key Redemption

Keys no longer store runtime HWID or own the license expiry. A key is redeemed once:

1. Client logs in and receives a JWT with `session_id`.
2. Client submits a key to `/loader/keys/activate`.
3. Server requires key status `NEW`.
4. Server adds the key type `duration_minutes` to `loader_accounts.expires_at`.
5. Server marks the key `ACTIVATED`.
6. Server records the redemption in `loader_key_redemptions`.

Used keys return:

```json
{
  "status": "KEY_ALREADY_USED",
  "active": false
}
```

## Single Active Session

Each successful login/register generates a new `session_id` and writes it to:

```text
loader_accounts.current_session_id
loader_accounts.current_hwid
loader_accounts.current_login_at
loader_accounts.last_seen_at
```

The JWT includes the same `session_id` and `hwid`. Protected `/loader/*` endpoints compare JWT values with the account row. If they differ, the old client receives:

```json
{
  "status": "SESSION_REPLACED",
  "active": false
}
```

The client must log out immediately on `SESSION_REPLACED`.

## Signed Responses

`/loader/keys/activate` and `/loader/heartbeat` return:

```json
{
  "signature_version": 2,
  "signed_payload": "{\"v\":2,\"action\":\"heartbeat\",...}",
  "signature": "RSA_SIGNATURE_BASE64"
}
```

Client verification:

- Verify `signature` over the exact `signed_payload`.
- Parse `signed_payload` only after verification succeeds.
- Require `v = 2`.
- Require `action` to match endpoint.
- Require `session_id`, `hwid`, and `nonce` to match the current request/session.
- Require `device_policy = "SINGLE_ACTIVE_SESSION"`.
- Require `issued_at` age <= 300 seconds.
- Trust `status`, `active`, `expiry`, and `remaining_seconds` only from the verified payload.

## Nonce Replay Protection

Every signed request must include a nonce:

- Length: `32-128` chars.
- Charset: `A-Z`, `a-z`, `0-9`, `_`, `-`.
- Nonces are hashed and stored in `public_api_nonces`.

Reused nonce returns `REPLAY_DETECTED`.

## Server Secrets

Required Cloudflare secrets:

```text
JWT_SECRET
RSA_PRIVATE_KEY
```

Never put any server secret in docs, client code, frontend bundles, or logs.

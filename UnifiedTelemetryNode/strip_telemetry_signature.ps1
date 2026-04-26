#
# strip_signature.ps1
# Post-build PE signature manipulation.
# 1. Strips the Rich header (compiler/linker version fingerprint)
# 2. Randomizes the PE TimeDateStamp to a plausible past date
#

param(
    [Parameter(Mandatory=$true)]
    [string]$TargetPath
)

if (-not (Test-Path $TargetPath)) {
    Write-Error "[-] Target not found: $TargetPath"
    exit 1
}

$bytes = [System.IO.File]::ReadAllBytes($TargetPath)

# ============================================================
# 1. Strip Rich Header
# ============================================================
# The Rich header sits between the DOS stub (ends around 0x80)
# and the PE signature. It ends with "Rich" + 4-byte XOR key.
# We find "Rich" (0x52 0x69 0x63 0x68) and zero everything
# from 0x80 up to and including the Rich marker + key (8 bytes).

$richOffset = -1
$searchLimit = [Math]::Min($bytes.Length, 0x400)

for ($i = 0x80; $i -lt ($searchLimit - 4); $i++) {
    if ($bytes[$i]   -eq 0x52 -and   # R
        $bytes[$i+1] -eq 0x69 -and   # i
        $bytes[$i+2] -eq 0x63 -and   # c
        $bytes[$i+3] -eq 0x68) {     # h
        $richOffset = $i
        break
    }
}

if ($richOffset -gt 0) {
    $endRich = $richOffset + 8  # "Rich" (4) + XOR key (4)
    for ($i = 0x80; $i -lt $endRich; $i++) {
        $bytes[$i] = 0
    }
    $zeroed = $endRich - 0x80
    Write-Host "[+] Rich header stripped ($zeroed bytes zeroed at 0x80-0x$($endRich.ToString('X')))"
} else {
    Write-Host "[!] Rich header not found (may already be stripped)"
}

# ============================================================
# 2. Randomize PE TimeDateStamp
# ============================================================
# e_lfanew at DOS header offset 0x3C gives the PE signature offset.
# COFF header starts at PE_sig + 4. TimeDateStamp is at COFF + 4.
# So: file offset = e_lfanew + 4 (skip "PE\0\0") + 4 (skip Machine field which is 2 bytes + NumberOfSections which is 2 bytes)
# Actually: COFF header = e_lfanew + 4. TimeDateStamp = COFF + 4.
# Machine (2 bytes) + NumberOfSections (2 bytes) = 4 bytes before TimeDateStamp.

$e_lfanew = [BitConverter]::ToInt32($bytes, 0x3C)
$timestampOffset = $e_lfanew + 8  # PE sig (4) + Machine (2) + NumSections (2)

$rng = [System.Random]::new()

# Generate timestamp between 2020-01-01 and 2024-06-01 (plausible build date)
$minEpoch = 1577836800   # 2020-01-01 UTC
$maxEpoch = 1717200000   # 2024-06-01 UTC
$newTimestamp = $rng.Next($minEpoch, $maxEpoch)

$tsBytes = [BitConverter]::GetBytes([int]$newTimestamp)
for ($i = 0; $i -lt 4; $i++) {
    $bytes[$timestampOffset + $i] = $tsBytes[$i]
}

$dt = (Get-Date "1970-01-01").AddSeconds($newTimestamp)
Write-Host "[+] PE timestamp randomized to $newTimestamp ($dt UTC)"

# ============================================================
# 3. Write modified binary back
# ============================================================
[System.IO.File]::WriteAllBytes($TargetPath, $bytes)
Write-Host "[+] Signature changes applied to: $TargetPath"

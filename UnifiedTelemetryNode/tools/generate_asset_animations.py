from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageFilter


FRAMES = 8
FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211


def lower_ascii(value: str) -> str:
    out = []
    for ch in value:
        code = ord(ch)
        if 65 <= code <= 90:
            out.append(chr(code + 32))
        else:
            out.append(ch)
    return "".join(out)


def normalize_resource_path(path: str) -> str:
    normalized = path.replace("\\", "/")
    lowered = lower_ascii(normalized)
    for root in ("assetsanimated/", "assets/", "datamacro/"):
        pos = lowered.find(root)
        if pos >= 0:
            normalized = normalized[pos:]
            lowered = lowered[pos:]
            break

    while lowered.startswith("./"):
        normalized = normalized[2:]
        lowered = lowered[2:]
    while lowered.startswith("/"):
        normalized = normalized[1:]
        lowered = lowered[1:]

    return lower_ascii(normalized)


def resource_name_for_path(path: str) -> str:
    value = FNV_OFFSET
    for byte in normalize_resource_path(path).encode("utf-8"):
        value ^= byte
        value = (value * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return f"ASSET_{value:016X}"


def build_motion_frame(base: Image.Image, cell_size: tuple[int, int],
                       scale: float, angle: float, offset: tuple[int, int]) -> Image.Image:
    width, height = base.size
    cell_w, cell_h = cell_size
    scaled_w = max(1, int(round(width * scale)))
    scaled_h = max(1, int(round(height * scale)))
    scaled = base.resize((scaled_w, scaled_h), Image.Resampling.LANCZOS)
    rotated = scaled.rotate(angle, resample=Image.Resampling.BICUBIC, expand=True)

    icon_layer = Image.new("RGBA", (cell_w, cell_h), (0, 0, 0, 0))
    x = (cell_w - rotated.width) // 2 + offset[0]
    y = (cell_h - rotated.height) // 2 + offset[1]
    icon_layer.alpha_composite(rotated, (x, y))

    shadow_alpha = icon_layer.getchannel("A").filter(ImageFilter.GaussianBlur(max(1, min(5, max(width, height) // 28))))
    shadow_alpha = shadow_alpha.point(lambda px: int(px * 0.34))
    shifted_shadow = Image.new("L", (cell_w, cell_h), 0)
    shifted_shadow.paste(shadow_alpha, (2, 3))
    shadow = Image.new("RGBA", (cell_w, cell_h), (0, 0, 0, 0))
    shadow.putalpha(shifted_shadow)

    return Image.alpha_composite(shadow, icon_layer)


def generate_sheet(source: Path, target: Path, force: bool) -> bool:
    if target.exists() and not force and target.stat().st_mtime >= source.stat().st_mtime:
        return False

    with Image.open(source) as opened:
        base = opened.convert("RGBA")

    width, height = base.size
    padding = min(96, max(6, int(max(width, height) * 0.14)))
    cell_w = width + padding * 2
    cell_h = height + padding * 2
    sheet = Image.new("RGBA", (cell_w * FRAMES, cell_h), (0, 0, 0, 0))
    motion = max(2, min(14, int(max(width, height) * 0.055)))
    scales = (0.86, 0.92, 1.00, 1.08, 1.02, 0.94, 0.88, 0.96)
    angles = (-7.0, -4.0, 0.0, 4.0, 7.0, 3.0, -2.0, -5.0)
    offsets = (
        (-0.65, 0.15),
        (-0.38, -0.55),
        (0.00, -0.80),
        (0.48, -0.45),
        (0.70, 0.10),
        (0.36, 0.58),
        (-0.10, 0.82),
        (-0.52, 0.50),
    )

    for frame_index in range(FRAMES):
        offset = (
            int(round(offsets[frame_index][0] * motion)),
            int(round(offsets[frame_index][1] * motion)),
        )
        frame = build_motion_frame(base, (cell_w, cell_h), scales[frame_index], angles[frame_index], offset)
        sheet.alpha_composite(frame, (frame_index * cell_w, 0))

    target.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(target, "PNG", optimize=True)
    return True


def write_resource_file(repo_root: Path) -> None:
    resource_roots = [repo_root / "Assets", repo_root / "AssetsAnimated", repo_root / "dataMacro"]
    entries: list[tuple[str, str]] = []
    for root in resource_roots:
        if not root.exists():
            continue
        for file_path in sorted((p for p in root.rglob("*") if p.is_file()), key=lambda p: p.as_posix().lower()):
            rel = file_path.relative_to(repo_root).as_posix()
            entries.append((resource_name_for_path(rel), f"../{rel}"))

    rc_path = repo_root / "telemetry" / "embedded_assets.rc"
    with rc_path.open("w", encoding="utf-8", newline="\n") as rc:
        rc.write("#pragma code_page(65001)\n\n")
        for name, rel in entries:
            rc.write(f'{name} RCDATA "{rel}"\n')


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate animated sprite sheets for Assets PNG files.")
    parser.add_argument("--force", action="store_true", help="Regenerate every sprite sheet.")
    parser.add_argument("--no-rc", action="store_true", help="Do not rebuild telemetry/embedded_assets.rc.")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    assets_root = repo_root / "Assets"
    animated_root = repo_root / "AssetsAnimated"

    total = 0
    updated = 0
    for source in sorted(assets_root.rglob("*.png"), key=lambda p: p.as_posix().lower()):
        rel = source.relative_to(assets_root)
        target = animated_root / rel
        total += 1
        if generate_sheet(source, target, args.force):
            updated += 1

    if not args.no_rc:
        write_resource_file(repo_root)

    print(f"generated {updated}/{total} animated sprite sheets into {animated_root}")
    if not args.no_rc:
        print("updated telemetry/embedded_assets.rc")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

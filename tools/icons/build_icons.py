"""Dựng 5 file .ico của TechVKey từ bản vector (không dùng ảnh raster nào làm nguồn).

  python tools/icons/build_icons.py                  # ghi thẳng vào Sources/OpenKey/win32/OpenKey/OpenKey
  python tools/icons/build_icons.py --out thu_muc    # ghi ra thư mục khác để xem thử trước
  python tools/icons/build_icons.py --check DIR      # chỉ kiểm tra các .ico trong DIR (không cần skia)

Mỗi kích thước được VẼ TRỰC TIẾP (cỡ 16-64 vẽ lớn gấp 4 rồi thu nhỏ cho mép mịn), không thu nhỏ từ bản 256:
cỡ nhỏ có viền và nét cyan dày hơn, khe chữ E nới ra để chữ còn đọc được ở khay hệ thống (xem OPTS).
"""
import argparse
import io
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageOps

HERE = Path(__file__).resolve().parent
DEFAULT_OUT = HERE.parents[1] / 'Sources/OpenKey/win32/OpenKey/OpenKey'
NAMES = ['icon', 'StatusViet', 'StatusEng', 'StatusViet10', 'StatusEng10']
SIZES = [256, 128, 64, 48, 40, 32, 24, 20, 16]

# Thông số theo kích thước (đơn vị thiết kế = pixel ảnh gốc x 3):
#   w: bề dày viền xanh của chữ trắng      rim: bề dày viền cyan của chữ xanh đậm
#   g_sw / g_glyph: nở nét cyan / chữ V của keycap       slot: nới khe chữ E       pad: lề quanh biểu tượng
#   detail: vẽ các đường sáng/tối mảnh (chỉ có nghĩa ở cỡ lớn)
OPTS = {
    256: dict(w=28, rim=15, g_sw=0, g_glyph=0, pad=0.035, detail=True),
    128: dict(w=28, rim=15, g_sw=0, g_glyph=0, pad=0.035, detail=True),
    64:  dict(w=32, rim=18, g_sw=0, g_glyph=0, pad=0.03, detail=True),
    48:  dict(w=38, rim=24, g_sw=4, g_glyph=0, pad=0.025, detail=False),
    40:  dict(w=44, rim=28, g_sw=8, g_glyph=6, pad=0.02, detail=False),
    32:  dict(w=50, rim=34, g_sw=12, g_glyph=10, pad=0.015, detail=False, slot=6),
    24:  dict(w=58, rim=40, g_sw=16, g_glyph=16, pad=0.012, detail=False, slot=14),
    20:  dict(w=64, rim=44, g_sw=18, g_glyph=20, pad=0.01, detail=False, slot=18),
    16:  dict(w=68, rim=48, g_sw=22, g_glyph=24, pad=0.01, detail=False, slot=22),
}
SUPERSAMPLE_UP_TO = 64


# ------------------------------------------------------------------ ghi ICO
def premul_resize(img, size):
    """Thu nhỏ trên ảnh nhân sẵn alpha để mép không bị viền màu lạ."""
    arr = np.asarray(img).astype(np.float32)
    a = arr[..., 3:4] / 255.0
    pm = Image.fromarray(np.dstack([arr[..., :3] * a, arr[..., 3:4]]).astype(np.uint8), 'RGBA').resize((size, size), Image.LANCZOS)
    p = np.asarray(pm).astype(np.float32)
    a2 = p[..., 3:4] / 255.0
    rgb = np.where(a2 > 0, p[..., :3] / np.maximum(a2, 1e-6), 0)
    return Image.fromarray(np.dstack([np.clip(rgb, 0, 255), p[..., 3:4]]).astype(np.uint8), 'RGBA')


def _png(img):
    buf = io.BytesIO()
    img.save(buf, 'PNG', optimize=True)
    return buf.getvalue()


def _dib(img):
    """Khung ICO dạng DIB 32 bit (BGRA, alpha thẳng) + mặt nạ AND rỗng, như icon Windows thông thường."""
    w, h = img.size
    bgra = ImageOps.flip(img).tobytes('raw', 'BGRA')
    mask = bytes(((w + 31) // 32 * 4) * h)
    return struct.pack('<IiiHHIIiiII', 40, w, 2 * h, 1, 32, 0, len(bgra) + len(mask), 0, 0, 0, 0) + bgra + mask


def write_ico(path, frames):
    """frames: [(kích thước, Image RGBA)]; khung 256 lưu PNG, các khung còn lại lưu DIB."""
    n = len(frames)
    entries, blobs, offset = b'', b'', 6 + 16 * n
    for size, img in frames:
        data = _png(img) if size >= 256 else _dib(img)
        entries += struct.pack('<BBBBHHII', size % 256, size % 256, 0, 0, 1, 32, len(data), offset)
        blobs += data
        offset += len(data)
    Path(path).write_bytes(struct.pack('<HHH', 0, 1, n) + entries + blobs)


# ------------------------------------------------------------------ dựng
def frame(name, size):
    import keycap_icon
    import status_icons
    opts = OPTS[size]
    scale = 4 if size <= SUPERSAMPLE_UP_TO else 1
    arr = keycap_icon.render(size * scale, opts) if name == 'icon' else status_icons.render(name, size * scale, opts)
    img = Image.fromarray(arr, 'RGBA')
    return img if scale == 1 else premul_resize(img, size)


def build(out):
    out.mkdir(parents=True, exist_ok=True)
    for name in NAMES:
        write_ico(out / (name + '.ico'), [(s, frame(name, s)) for s in SIZES])
        print('đã ghi', out / (name + '.ico'))


# ------------------------------------------------------------------ kiểm tra
def _exterior(alpha):
    """Vùng trong suốt nối với mép ảnh (lan dần từ mép vào, chỉ qua pixel gần như trong suốt)."""
    clear = alpha < 8
    region = np.zeros_like(clear)
    region[0, :], region[-1, :], region[:, 0], region[:, -1] = clear[0, :], clear[-1, :], clear[:, 0], clear[:, -1]
    while True:
        grown = region.copy()
        grown[1:, :] |= region[:-1, :]
        grown[:-1, :] |= region[1:, :]
        grown[:, 1:] |= region[:, :-1]
        grown[:, :-1] |= region[:, 1:]
        grown &= clear
        if (grown == region).all():
            return region
        region = grown


def fringe_ratio(rgba):
    """Tỉ lệ pixel gần trắng trong dải 1-2px sát mép ngoài. Bộ icon cũ cắt từ ảnh render dính viền trắng ở đây."""
    a, rgb = rgba[..., 3], rgba[..., :3].astype(int)
    outside = _exterior(a)
    ring = ~outside & (a > 0)
    near = outside.copy()
    for _ in range(2):                                          # mở rộng vùng ngoài thêm 2px để lấy dải biên
        g = near.copy()
        g[1:, :] |= near[:-1, :]
        g[:-1, :] |= near[1:, :]
        g[:, 1:] |= near[:, :-1]
        g[:, :-1] |= near[:, 1:]
        near = g
    ring &= near
    light = (rgb.min(-1) > 200) & (rgb.max(-1) - rgb.min(-1) < 45)
    return float((ring & light).sum()) / max(int(ring.sum()), 1)


def check(directory):
    problems = []
    for name in NAMES:
        path = directory / (name + '.ico')
        if not path.exists():
            problems.append('%s: thiếu tệp' % path.name)
            continue
        ico = Image.open(path)
        sizes = sorted(s[0] for s in ico.info['sizes'])
        if sizes != sorted(SIZES):
            problems.append('%s: cỡ %s, cần %s' % (path.name, sizes, sorted(SIZES)))
            continue
        for s in SIZES:
            frame_rgba = np.asarray(ico.ico.getimage((s, s)).convert('RGBA'))
            a = frame_rgba[..., 3]
            if max(a[0, 0], a[0, -1], a[-1, 0], a[-1, -1]) > 16:        # alpha 1-5 ở góc chỉ là dư khử răng cưa
                problems.append('%s %dpx: góc ảnh không trong suốt' % (path.name, s))
            if (a > 0).mean() < 0.2:
                problems.append('%s %dpx: hình quá nhỏ trong khung' % (path.name, s))
        ratio = fringe_ratio(np.asarray(ico.ico.getimage((256, 256)).convert('RGBA')))
        if ratio > 0.01:
            problems.append('%s: viền ngoài có %.1f%% pixel gần trắng (nghi dính viền nền)' % (path.name, ratio * 100))
    return problems


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--out', type=Path, default=DEFAULT_OUT, help='thư mục ghi .ico (mặc định: thư mục icon của project Windows)')
    ap.add_argument('--check', type=Path, metavar='DIR', help='chỉ kiểm tra các .ico trong DIR')
    args = ap.parse_args()
    if args.check is None:
        sys.path.insert(0, str(HERE))
        build(args.out)
    problems = check(args.check or args.out)
    for p in problems:
        print('LỖI:', p)
    print('Kiểm tra: %s' % ('có %d vấn đề' % len(problems) if problems else 'đạt'))
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())

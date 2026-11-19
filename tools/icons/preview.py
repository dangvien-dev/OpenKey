"""Ảnh xem thử các .ico trước khi commit: nền trắng/đen ở cỡ lớn, và cỡ khay 16/20/24/32 trên taskbar sáng/tối.

  python tools/icons/preview.py [THU_MUC_ICO] [TIEN_TO_ANH]      -> TIEN_TO_ANH_big.png, TIEN_TO_ANH_tray.png
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
DEFAULT_DIR = HERE / 'out'
NAMES = ['icon', 'StatusViet', 'StatusEng', 'StatusViet10', 'StatusEng10']
TRAY_SIZES = [16, 20, 24, 32]
LIGHT_TASKBAR, DARK_TASKBAR = (243, 243, 243), (32, 32, 32)


def font(size):
    try:
        return ImageFont.truetype('DejaVuSans.ttf', size)
    except OSError:
        return ImageFont.load_default()


def frame(directory, name, size):
    return Image.open(Path(directory) / (name + '.ico')).ico.getimage((size, size)).convert('RGBA')


def on(img, bg):
    out = Image.new('RGB', img.size, bg)
    out.paste(img, (0, 0), img)
    return out


def big_sheet(directory, out):
    cell = 230
    sheet = Image.new('RGB', (len(NAMES) * (cell + 10) + 10, 2 * (cell + 10) + 40), (120, 120, 120))
    draw = ImageDraw.Draw(sheet)
    for i, name in enumerate(NAMES):
        img = frame(directory, name, 256).resize((cell, cell), Image.LANCZOS)
        sheet.paste(on(img, (255, 255, 255)), (10 + i * (cell + 10), 36))
        sheet.paste(on(img, (0, 0, 0)), (10 + i * (cell + 10), 36 + cell + 10))
        draw.text((12 + i * (cell + 10), 8), name, fill=(255, 255, 255), font=font(15))
    sheet.save(out)


def tray_sheet(directory, out, zoom=5):
    cell = 32 * zoom + 12
    rows = [(bg, label, name) for bg, label in ((LIGHT_TASKBAR, 'taskbar sáng'), (DARK_TASKBAR, 'taskbar tối')) for name in NAMES[1:]]
    sheet = Image.new('RGB', (len(TRAY_SIZES) * cell + 330, len(rows) * (cell + 8) + 50), (90, 90, 90))
    draw = ImageDraw.Draw(sheet)
    y = 40
    for bg, label, name in rows:
        draw.rectangle([0, y - 4, sheet.width, y + cell], fill=bg)
        draw.text((8, y + cell // 2 - 10), '%s\n%s' % (label, name), fill=(0, 0, 0) if bg[0] > 128 else (255, 255, 255), font=font(15))
        x = 170 + len(TRAY_SIZES) * cell + 6
        for i, s in enumerate(TRAY_SIZES):
            img = frame(directory, name, s)
            sheet.paste(on(img.resize((s * zoom, s * zoom), Image.NEAREST), bg), (170 + i * cell, y))     # phóng to để soi từng pixel
            sheet.paste(on(img, bg), (x, y + 8))                                                           # kích thước thật
            x += s + 8
        y += cell + 8
    sheet.save(out)


if __name__ == '__main__':
    directory = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_DIR
    prefix = sys.argv[2] if len(sys.argv) > 2 else 'preview'
    big_sheet(directory, prefix + '_big.png')
    tray_sheet(directory, prefix + '_tray.png')
    print('đã ghi %s_big.png và %s_tray.png' % (prefix, prefix))

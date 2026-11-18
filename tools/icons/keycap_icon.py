"""icon.ico: phím V nghiêng (keycap). Toạ độ thiết kế = ảnh approved_design_source.png cắt vùng phím rồi phóng 3 lần."""
import numpy as np

import skia

import draw as d

BASE = ([(190, 240), (930, 260), (1204, 940), (10, 1026)], [100, 100, 170, 150])                              # đế xanh
KEY = ([(208, 172), (864, 22), (1068, 610), (1100, 827), (268, 960), (160, 400)], [110, 90, 60, 85, 80, 100])  # thân phím (viền vát + sườn)
PLATEAU = ([(206, 200), (868, 62), (1058, 682), (338, 799)], [95, 120, 90, 100])                              # mặt trên phím
GLYPH_V = ([(352, 388), (492, 343), (598, 562), (700, 280), (842, 250), (664, 692), (542, 714)], [16, 16, 8, 16, 16, 22, 22])
HAT_OUTER = [(515, 290), (565, 262), (620, 205), (665, 152), (695, 138), (735, 158), (790, 190), (832, 192)]  # dấu mũ
HAT_INNER = [(832, 192), (790, 206), (745, 208), (715, 192), (690, 202), (650, 238), (590, 276), (515, 290)]
SW_OUTER = [(735, 728), (850, 704), (960, 664), (1040, 610), (1100, 545), (1130, 470), (1118, 400), (1082, 345), (1030, 315), (975, 305)]
SW_INNER = [(735, 728), (850, 692), (935, 640), (985, 578), (995, 505), (988, 440), (980, 380), (975, 335), (975, 305)]


def render(size, opts):
    surface, c = d.new_canvas(size)
    base = d.rounded_polygon(*BASE)
    key = d.rounded_polygon(*KEY)
    plateau = d.rounded_polygon(*PLATEAU)
    outer, inner = d.catmull(SW_OUTER), d.catmull(SW_INNER)
    crescent = d.polyline_path(np.vstack([outer, inner[::-1]]))
    rects = [base.getBounds(), crescent.getBounds(), key.getBounds()]
    d.fit(c, size, (min(r.left() for r in rects), min(r.top() for r in rects), max(r.right() for r in rects), max(r.bottom() for r in rects)), opts['pad'])

    # đế xanh: viền sáng bên ngoài + lòng đậm bên trong
    d.fill(c, base, shader=d.gradient((20, 500), (1200, 900), [(0, '#0b6cf6'), (0.3, '#0a55dc'), (1, '#0a46c8')]))
    d.fill(c, d.inset(base, 52), shader=d.gradient((100, 300), (1100, 1000), [(0, '#0a49cc'), (0.5, '#0a40b8'), (1, '#082f9a')]))
    # thân phím; viền xanh mảnh để mép phím không tan vào nền trắng
    d.fill(c, key, shader=d.gradient((0, 560), (0, 960), [(0, '#eaeffb'), (0.3, '#dbe4f6'), (0.55, '#bccbee'), (1, '#8da8de')]))
    d.stroke(c, key, 10, '#7b97d8')
    # mặt trên + đường sáng quanh mép
    d.fill(c, plateau, shader=d.gradient((200, 60), (1000, 800), [(0, '#f6f9fe'), (0.6, '#e8eefb'), (1, '#d4dff4')]))
    d.stroke(c, plateau, 9, '#ffffff')

    # chữ V và dấu mũ (nở ra ở cỡ nhỏ cho đậm nét)
    g = opts.get('g_glyph', 0)
    d.fill(c, d.grown(d.rounded_polygon(*GLYPH_V), g), shader=d.gradient((0, 250), (0, 716), [(0, '#1558cc'), (0.5, '#0a42b4'), (1, '#0a2d96')]))
    hat = d.grown(d.polyline_path(np.vstack([d.catmull(HAT_OUTER), d.catmull(HAT_INNER)])), g)
    d.fill(c, hat, shader=d.gradient((0, 140), (0, 290), [(0, '#1558cc'), (1, '#0a3aa8')]))

    # nét cyan phía trước
    crescent = d.grown(crescent, opts.get('g_sw', 0))
    d.fill(c, crescent, shader=d.gradient((735, 728), (1110, 380), [(0, '#18f0ff'), (0.35, '#16d6fb'), (0.7, '#0e8cf0'), (1, '#0a55d6')]))
    dist = np.linalg.norm(outer[:, None, :] - inner[None, :, :], axis=2)
    mid = outer + (inner[dist.argmin(axis=1)] - outer) * 0.3
    d.fill(c, d.polyline_path(np.vstack([outer, mid[::-1]])), shader=d.gradient((735, 728), (1110, 380), [(0, '#10b6f8'), (0.5, '#0b78e6'), (1, '#0a48c4')]))
    c.save()
    c.clipPath(crescent, doAntiAlias=True)
    d.stroke(c, d.polyline_path(inner, close=False), 7, '#9ff7ff', cap=skia.Paint.kButt_Cap)
    glass = skia.Paint(AntiAlias=True, Style=skia.Paint.kStroke_Style, StrokeWidth=34, StrokeCap=skia.Paint.kButt_Cap, Color=skia.Color(214, 230, 255, 110))
    c.drawPath(d.polyline_path(inner[len(inner) // 2:], close=False), glass)        # dải kính mờ nơi nét đè lên cạnh phím
    c.restore()
    return d.snapshot(surface)

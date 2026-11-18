"""Hàm vẽ dùng chung cho bộ icon: đa giác bo góc, đường cong mượt, thu/nở viền, tô gradient (Skia)."""
import math

import numpy as np
import skia


def rgb(hex_color, alpha=255):
    h = hex_color.lstrip('#')
    return skia.Color(int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16), alpha)


def rounded_polygon(points, radii):
    """Đa giác mà mỗi đỉnh được bo bằng cung tròn (xấp xỉ cubic) với bán kính riêng.

    `points` là các ĐỈNH NHỌN ảo của hình; cung bo nằm bên trong chúng.
    """
    n = len(points)
    pts = [np.array(p, dtype=float) for p in points]
    segs = []
    for i in range(n):
        p0, p1, p2 = pts[i - 1], pts[i], pts[(i + 1) % n]
        u, v = p0 - p1, p2 - p1
        lu, lv = np.linalg.norm(u), np.linalg.norm(v)
        u, v = u / lu, v / lv
        theta = math.acos(max(-1.0, min(1.0, float(u @ v))))          # góc trong tại đỉnh
        r = radii[i] if hasattr(radii, '__getitem__') else radii
        t = min(r / math.tan(theta / 2), lu * 0.5, lv * 0.5)          # chiều dài tiếp tuyến
        r_eff = t * math.tan(theta / 2)
        handle = 4.0 / 3.0 * math.tan((math.pi - theta) / 4) * r_eff   # tay cầm của cubic xấp xỉ cung tròn
        a, b = p1 + u * t, p1 + v * t
        segs.append((a, a - u * handle, b - v * handle, b))
    path = skia.Path()
    path.moveTo(*segs[0][3])
    for i in range(1, n + 1):
        a, c1, c2, b = segs[i % n]
        path.lineTo(*a)
        path.cubicTo(*c1, *c2, *b)
    path.close()
    return path


def catmull(points, per=48):
    """Đường cong Catmull-Rom (centripetal) đi qua các điểm; trả về mảng (N, 2) điểm dày."""
    p = [np.array(q, dtype=float) for q in points]
    p = [2 * p[0] - p[1]] + p + [2 * p[-1] - p[-2]]
    out = []
    for i in range(1, len(p) - 2):
        p0, p1, p2, p3 = p[i - 1], p[i], p[i + 1], p[i + 2]
        t0 = 0.0
        t1 = t0 + max(np.linalg.norm(p1 - p0), 1e-6) ** 0.5
        t2 = t1 + max(np.linalg.norm(p2 - p1), 1e-6) ** 0.5
        t3 = t2 + max(np.linalg.norm(p3 - p2), 1e-6) ** 0.5
        for t in np.linspace(t1, t2, per, endpoint=False):
            a1 = (t1 - t) / (t1 - t0) * p0 + (t - t0) / (t1 - t0) * p1
            a2 = (t2 - t) / (t2 - t1) * p1 + (t - t1) / (t2 - t1) * p2
            a3 = (t3 - t) / (t3 - t2) * p2 + (t - t2) / (t3 - t2) * p3
            b1 = (t2 - t) / (t2 - t0) * a1 + (t - t0) / (t2 - t0) * a2
            b2 = (t3 - t) / (t3 - t1) * a2 + (t - t1) / (t3 - t1) * a3
            out.append((t2 - t) / (t2 - t1) * b1 + (t - t1) / (t2 - t1) * b2)
    out.append(p[-2])
    return np.array(out)


def polyline_path(points, close=True):
    path = skia.Path()
    path.moveTo(*points[0])
    prev = np.asarray(points[0], dtype=float)
    for q in points[1:]:
        if np.linalg.norm(np.asarray(q, dtype=float) - prev) < 1e-3:   # bỏ điểm trùng: Skia Op lỗi với đoạn suy biến
            continue
        prev = np.asarray(q, dtype=float)
        path.lineTo(*q)
    if close:
        path.close()
    return path


def swoosh_shapes(outer, inner, band=0.36):
    """Nét cyan: lưỡi liềm giữa đường ngoài `outer` và đường trong `inner` (cùng đi từ mũi trái sang mũi phải).

    Trả về (lưỡi liềm, dải xanh dọc mép ngoài, đường mép trong).
    """
    o, i = catmull(outer), catmull(inner)
    crescent = polyline_path(np.vstack([o, i[::-1]]))
    dist = np.linalg.norm(o[:, None, :] - i[None, :, :], axis=2)
    mid = o + (i[dist.argmin(axis=1)] - o) * band                      # dải hẹp dần về hai đầu mũi
    return crescent, polyline_path(np.vstack([o, mid[::-1]])), polyline_path(i, close=False)


def inset(path, width):
    """Thu hình vào `width`: lấy hình trừ đi dải viền rộng 2*width quanh biên."""
    paint = skia.Paint(Style=skia.Paint.kStroke_Style, StrokeWidth=2 * width, StrokeJoin=skia.Paint.kRound_Join, AntiAlias=True)
    stroked = skia.Path()
    paint.getFillPath(path, stroked)
    return skia.Op(path, stroked, skia.PathOp.kDifference_PathOp)


def grown(path, amount):
    """Nở hình ra ngoài `amount` (cỡ nhỏ cần nét dày hơn)."""
    if not amount:
        return path
    paint = skia.Paint(Style=skia.Paint.kStrokeAndFill_Style, StrokeWidth=2 * amount, StrokeJoin=skia.Paint.kRound_Join, AntiAlias=True)
    out = skia.Path()
    paint.getFillPath(path, out)
    try:
        return skia.Simplify(out) or out
    except RuntimeError:           # đường tự cắt: Skia không đơn giản hoá được, dùng nguyên (vẽ vẫn đúng)
        return out


def gradient(p0, p1, stops):
    return skia.GradientShader.MakeLinear([p0, p1], [rgb(c) for _, c in stops], [t for t, _ in stops])


def fill(canvas, path, shader=None, color=None):
    paint = skia.Paint(AntiAlias=True)
    if shader is not None:
        paint.setShader(shader)
    else:
        paint.setColor(rgb(color))
    canvas.drawPath(path, paint)


def stroke(canvas, path, width, color, cap=skia.Paint.kRound_Cap):
    paint = skia.Paint(AntiAlias=True, Style=skia.Paint.kStroke_Style, StrokeWidth=width, StrokeCap=cap, StrokeJoin=skia.Paint.kRound_Join)
    paint.setColor(rgb(color))
    canvas.drawPath(path, paint)


def new_canvas(size):
    surface = skia.Surface(size, size)
    canvas = surface.getCanvas()
    canvas.clear(skia.ColorTRANSPARENT)
    return surface, canvas


def fit(canvas, size, bounds, pad):
    """Căn `bounds` (x0, y0, x1, y1) vào giữa canvas vuông, chừa lề `pad` (tỉ lệ cạnh)."""
    x0, y0, x1, y1 = bounds
    s = size * (1 - 2 * pad) / max(x1 - x0, y1 - y0)
    canvas.translate(size / 2, size / 2)
    canvas.scale(s, s)
    canvas.translate(-(x0 + x1) / 2, -(y0 + y1) / 2)


def snapshot(surface):
    """Ảnh RGBA 8 bit, alpha thẳng (không nhân sẵn), đúng định dạng của khung ICO."""
    img = surface.makeImageSnapshot()
    return np.asarray(img.toarray(colorType=skia.kRGBA_8888_ColorType, alphaType=skia.kUnpremul_AlphaType))

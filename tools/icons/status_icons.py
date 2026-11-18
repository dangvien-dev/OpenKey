"""4 icon trạng thái: StatusViet / StatusEng (chữ trắng viền xanh) và StatusViet10 / StatusEng10 (chữ xanh đậm viền cyan).

Hình dạng đo từ approved_design_source.png. Toạ độ thiết kế = pixel của ảnh gốc nhân 3:
  - `letter`: các đỉnh NHỌN của chữ (đi vòng quanh, theo chiều kim đồng hồ), `radii`: bán kính bo của từng đỉnh
    (chữ do regular_v() / regular_e() dựng, xem ĐỘ ĐẬM CHỮ bên dưới);
  - `sw_outer` / `sw_inner`: hai đường cong của nét cyan, đi từ mũi trái sang mũi phải.
"""
import skia

import draw as d

# ---- ĐỘ ĐẬM CHỮ (đơn vị thiết kế; chiều cao chữ = 675 với V, 707 với E).
# Regular ~ 15% chiều cao chữ, Bold ~ 21%, Black ~ 29% (bản đầu từ ảnh ChatGPT là Black: nét dày 200-280, trông nặng).
# Đang dùng ~17-18% (Regular dày nhẹ): chữ trắng còn viền xanh hai bên nên nét 15% chỉ còn lõi trắng dưới 1 pixel ở cỡ khay.
V_STEM = 150     # bề ngang một nét của V (đo ngang, ở mép trên)
E_ARM = 125      # chiều dày ba nhánh ngang của E
E_SPINE = 150    # bề ngang gạch đứng của E


def regular_v():
    """V nghiêng độ đậm Regular: hai nét chéo gặp nhau ở đáy phẳng. Trả về (đỉnh nhọn, bán kính bo)."""
    top, bottom, left, right = 82, 757, 46, 879      # khung ngoài giữ như bản đầu
    bl, br = 380, 500                                # đáy phẳng hẹp (hai nét gặp nhau, không còn khối đáy dày)
    sl, sr = (bl - left) / (bottom - top), (br - right) / (bottom - top)
    t = ((right - V_STEM) - (left + V_STEM)) / (sl - sr)     # giao điểm hai cạnh trong = đáy chữ V
    notch = (left + V_STEM + sl * t, top + t)
    pts = [(left, top), (left + V_STEM, top), notch, (right - V_STEM, top), (right, top), (br, bottom), (bl, bottom)]
    return pts, [40, 22, 12, 22, 40, 35, 35]


def regular_e():
    """E nghiêng độ đậm Regular: gạch đứng + ba nhánh cùng dày, khe rộng. Trả về (đỉnh nhọn, bán kính bo)."""
    top, bottom, tl, tr, slope = 96, 803, 211, 834, 0.22     # khung ngoài + độ nghiêng giữ như bản đầu
    lean = (tl - 57) / (bottom - top)                        # độ nghiêng cạnh trái (x giảm dần xuống dưới)
    spine = lambda y: tl + E_SPINE - lean * (y - top)        # mép trong gạch đứng tại độ cao y
    right = lambda y, cut: tr - slope * (y - top) - cut      # mép phải nhánh (nhánh giữa ngắn hơn)
    gap = ((bottom - top) - 3 * E_ARM) / 2                   # chiều cao mỗi khe
    y1, y2 = top + E_ARM, top + E_ARM + gap                  # nhánh trên: [top, y1]; nhánh giữa: [y2, y2 + E_ARM]
    y3, y4 = y2 + E_ARM, bottom - E_ARM                      # nhánh dưới: [y4, bottom]
    pts = [(tl, top), (tr, top), (right(y1, 0), y1), (spine(y1), y1), (spine(y2), y2),
           (right(y2, 70), y2), (right(y3, 70), y3), (spine(y3), y3), (spine(y4), y4),
           (right(y4, 5), y4), (right(bottom, 5), bottom), (57, bottom)]
    return pts, [40, 35, 30, 8, 8, 30, 30, 8, 8, 30, 35, 40]


_V_LETTER = regular_v()
_E_LETTER = regular_e()

V_WHITE = dict(
    letter=_V_LETTER[0], radii=_V_LETTER[1],
    sw_outer=[(154, 839), (349, 842), (553, 809), (694, 752), (785, 676), (827, 607), (845, 532), (836, 451), (802, 396)],
    sw_inner=[(154, 839), (409, 774), (616, 660), (750, 535), (797, 440), (802, 396)],
)
E_WHITE = dict(
    letter=_E_LETTER[0], radii=_E_LETTER[1],
    sw_outer=[(210, 883), (391, 881), (604, 848), (751, 791), (823, 734), (875, 652), (893, 568), (881, 505), (829, 438)],
    sw_inner=[(210, 883), (444, 817), (727, 654), (815, 540), (829, 438)],
)
V_BLUE = dict(
    letter=_V_LETTER[0], radii=_V_LETTER[1],
    sw_outer=[(166, 848), (352, 857), (562, 827), (700, 773), (803, 682), (842, 610), (854, 547), (848, 475), (814, 408)],
    sw_inner=[(166, 848), (463, 771), (613, 684), (740, 570), (807, 445), (814, 408)],
)
E_BLUE = dict(
    letter=_E_LETTER[0], radii=_E_LETTER[1],
    sw_outer=[(195, 874), (385, 869), (568, 842), (697, 800), (784, 746), (842, 679), (878, 574), (869, 490), (817, 423)],
    sw_inner=[(195, 874), (504, 772), (715, 636), (810, 517), (817, 423)],
)


def _bounds(spec, with_swoosh=True):
    pts = spec['letter'] + (spec['sw_outer'] if with_swoosh else [])
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]
    return min(xs), min(ys), max(xs), max(ys)


def _paint_swoosh(canvas, spec, crescent, lower, edge, opts):
    lx, ly = spec['sw_outer'][0]
    rx, ry = spec['sw_outer'][-1]
    if opts.get('swoosh') == 'flat':                                   # cỡ khay: một mảng cyan đặc, không dải xanh/đường sáng
        d.fill(canvas, crescent, shader=d.gradient((lx, ly), (rx, ry), [(0, '#35dcfc'), (1, '#3fe6ff')]))
        return
    d.fill(canvas, crescent, shader=d.gradient((lx, ly), (rx, ry), [(0, '#38e7fe'), (0.35, '#5cf9ff'), (0.75, '#2fe0fb'), (1, '#16bff0')]))
    d.fill(canvas, lower, shader=d.gradient((lx, ly), (rx + 60, ry + 160), [(0, '#10a4f8'), (0.5, '#0988f2'), (1, '#0762d6')]))
    if opts.get('detail', True):                                      # đường sáng mảnh dọc mép trong (chỉ ở cỡ lớn)
        canvas.save()
        canvas.clipPath(crescent, doAntiAlias=True)
        d.stroke(canvas, edge, 8, '#a6f9ff', cap=skia.Paint.kButt_Cap)
        canvas.restore()


def _paint_white(canvas, letter, bb, opts):
    _, y0, _, y1 = bb
    d.fill(canvas, letter, shader=d.gradient((0, y0), (0, y1), [(0, '#0a56d0'), (0.5, '#0446b8'), (1, '#02349f')]))      # viền xanh
    inner = d.inset(letter, opts.get('w', 28))
    d.fill(canvas, inner, shader=d.gradient((0, y0), (0, y1), [(0, '#fcfdff'), (0.55, '#f4f8fd'), (1, '#e6edf9')]))      # ruột trắng
    if opts.get('detail', True):
        d.stroke(canvas, inner, 5, '#0134a8')                                                                              # đường tối mảnh sát mép trong


def _paint_blue(canvas, letter, bb, opts):
    _, y0, _, y1 = bb
    if opts.get('bright'):                  # cỡ khay: thân sáng hơn để không chìm vào taskbar tối, viền cyan nổi rõ
        rim = [(0, '#7af4ff'), (0.5, '#3cd0fb'), (1, '#27b4f6')]
        body = [(0, '#2f8ff8'), (0.5, '#1d73ea'), (1, '#1260d6')]
    else:
        rim = [(0, '#48eeff'), (0.3, '#20b4fa'), (1, '#0673df')]
        body = [(0, '#0a72f0'), (0.5, '#0356d2'), (1, '#0138a2')]
    d.fill(canvas, letter, shader=d.gradient((0, y0), (0, y1), rim))                                                    # viền cyan
    d.fill(canvas, d.inset(letter, opts.get('rim', 15)), shader=d.gradient((0, y0), (0, y1), body))                      # thân xanh


ICONS = {
    'StatusViet': (V_WHITE, _paint_white),
    'StatusEng': (E_WHITE, _paint_white),
    'StatusViet10': (V_BLUE, _paint_blue),
    'StatusEng10': (E_BLUE, _paint_blue),
}


def render(name, size, opts):
    """Vẽ icon `name` thành ảnh RGBA size x size. `opts`: xem OPTS trong build_icons.py."""
    spec, paint_letter = ICONS[name]
    surface, canvas = d.new_canvas(size)
    show_swoosh = opts.get('swoosh') != 'none'
    bb = _bounds(spec, show_swoosh)                 # không có nét cyan thì chữ chiếm trọn khung
    d.fit(canvas, size, bb, opts['pad'])

    pts = spec['letter']
    shift = opts.get('slot', 0) * spec.get('slot_scale', 1.0)
    if shift and 'slots' in spec:
        pts = [(x, y + spec['slots'][y] * shift) if y in spec['slots'] else (x, y) for x, y in pts]
    letter = d.grown(d.rounded_polygon(pts, spec['radii']), opts.get('g_letter', 0))
    crescent, lower, edge = d.swoosh_shapes(spec['sw_outer'], spec['sw_inner'])
    crescent, lower = d.grown(crescent, opts.get('g_sw', 0)), d.grown(lower, opts.get('g_sw', 0))

    if show_swoosh:
        _paint_swoosh(canvas, spec, crescent, lower, edge, opts)  # nét cyan nằm SAU chữ
    paint_letter(canvas, letter, bb, opts)
    return d.snapshot(surface)

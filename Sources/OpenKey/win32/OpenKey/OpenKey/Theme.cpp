/*----------------------------------------------------------
OpenKey - The Cross platform Open source Vietnamese Keyboard application.

Copyright (C) 2019 Mai Vu Tuyen
Contact: maivutuyen.91@gmail.com
Github: https://github.com/tuyenvm/OpenKey
Fanpage: https://www.facebook.com/OpenKeyVN

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#include "Theme.h"
#include <windowsx.h>
#include <objidl.h>
#include <gdiplus.h>
#include <Uxtheme.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "comctl32.lib")

namespace Theme {

namespace {

/* ---------- Window properties dùng để gắn metadata lên control ---------- */
const wchar_t* const kPropKind  = L"OKTheme.Kind";   // ButtonKind
const wchar_t* const kPropGlyph = L"OKTheme.Glyph";  // const wchar_t* (literal)
const wchar_t* const kPropRole  = L"OKTheme.Role";   // LabelRole
const wchar_t* const kPropCard  = L"OKTheme.Card";   // control nằm trên card trắng
const wchar_t* const kPropPage  = L"OKTheme.Page";   // dialog là trang của tab (nền trắng)

/* ---------- Kích thước @96dpi ---------- */
const int kTabHeaderPx   = 34;
const int kTabInsetPx    = 8;
const int kCardRadiusPx  = 8;
const int kButtonRadiusPx = 6;
const int kCheckBoxPx    = 16;
const int kCheckGapPx    = 8;
const int kCheckPadLeftPx = 2;

/* ---------- Trạng thái toàn cục ---------- */
bool      g_inited = false;
ULONG_PTR g_gdiplusToken = 0;
int       g_dpi = 96;
bool      g_hasIconFont = false;
HBRUSH    g_brBg = NULL;
HBRUSH    g_brCard = NULL;
int       g_enabledCache = -1;

struct FontEntry {
	LOGFONTW lf;
	HFONT font;
};

vector<FontEntry>& fontCache() {
	static vector<FontEntry> cache;
	return cache;
}

inline int rcW(const RECT& r) { return r.right - r.left; }
inline int rcH(const RECT& r) { return r.bottom - r.top; }

int CALLBACK enumFontFound(const LOGFONTW*, const TEXTMETRICW*, DWORD, LPARAM lp) {
	*(bool*)lp = true;
	return 0; // dừng ngay khi tìm thấy
}

bool fontInstalled(const wchar_t* face) {
	LOGFONTW lf = { 0 };
	lf.lfCharSet = DEFAULT_CHARSET;
	lstrcpynW(lf.lfFaceName, face, LF_FACESIZE);
	bool found = false;
	HDC dc = GetDC(NULL);
	EnumFontFamiliesExW(dc, &lf, enumFontFound, (LPARAM)&found, 0);
	ReleaseDC(NULL, dc);
	return found;
}

void ensureInit() {
	if (g_inited)
		return;
	g_inited = true;

	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &input, NULL);

	HDC dc = GetDC(NULL);
	g_dpi = GetDeviceCaps(dc, LOGPIXELSX);
	ReleaseDC(NULL, dc);
	if (g_dpi <= 0)
		g_dpi = 96;

	g_brBg = CreateSolidBrush(kBgMain);
	g_brCard = CreateSolidBrush(kCard);
	g_hasIconFont = fontInstalled(L"Segoe MDL2 Assets");
}

inline int px(int px96) { return MulDiv(px96, g_dpi, 96); }

/* ---------- Font ---------- */
HFONT controlFont(HWND h) {
	HFONT f = (HFONT)SendMessage(h, WM_GETFONT, 0, 0);
	return f ? f : (HFONT)GetStockObject(DEFAULT_GUI_FONT);
}

HFONT cachedFont(const LOGFONTW& want) {
	vector<FontEntry>& cache = fontCache();
	for (size_t i = 0; i < cache.size(); i++) {
		const LOGFONTW& e = cache[i].lf;
		if (e.lfHeight == want.lfHeight && e.lfWeight == want.lfWeight && e.lfItalic == want.lfItalic &&
			e.lfCharSet == want.lfCharSet && lstrcmpW(e.lfFaceName, want.lfFaceName) == 0)
			return cache[i].font;
	}
	HFONT f = CreateFontIndirectW(&want);
	if (f) {
		FontEntry entry;
		entry.lf = want;
		entry.font = f;
		cache.push_back(entry);
	}
	return f;
}

// Font semibold cùng cỡ với font gốc (Segoe UI Semibold có sẵn từ Vista).
HFONT semiboldOf(HFONT base) {
	LOGFONTW lf = { 0 };
	if (!GetObjectW(base, sizeof(lf), &lf))
		return base;
	lstrcpynW(lf.lfFaceName, L"Segoe UI Semibold", LF_FACESIZE);
	lf.lfWeight = FW_NORMAL;
	lf.lfQuality = CLEARTYPE_QUALITY;
	HFONT f = cachedFont(lf);
	return f ? f : base;
}

// Font Segoe UI cỡ `points` (pt), weight tùy chọn.
HFONT uiFont(int points, bool semibold) {
	LOGFONTW lf = { 0 };
	lf.lfHeight = -MulDiv(points, g_dpi, 72);
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lstrcpynW(lf.lfFaceName, semibold ? L"Segoe UI Semibold" : L"Segoe UI", LF_FACESIZE);
	return cachedFont(lf);
}

HFONT iconFont(int heightPx) {
	LOGFONTW lf = { 0 };
	lf.lfHeight = -heightPx;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lstrcpynW(lf.lfFaceName, L"Segoe MDL2 Assets", LF_FACESIZE);
	return cachedFont(lf);
}

int textWidth(HDC dc, HFONT font, const wchar_t* s) {
	HFONT old = (HFONT)SelectObject(dc, font);
	SIZE sz = { 0, 0 };
	GetTextExtentPoint32W(dc, s, lstrlenW(s), &sz);
	SelectObject(dc, old);
	return sz.cx;
}

void drawText(HDC dc, const wchar_t* s, RECT rc, UINT fmt, COLORREF color, HFONT font) {
	HFONT old = (HFONT)SelectObject(dc, font);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, color);
	DrawTextW(dc, s, -1, &rc, fmt | DT_NOPREFIX);
	SelectObject(dc, old);
}

bool isClass(HWND h, const wchar_t* name) {
	wchar_t buf[64];
	if (!GetClassNameW(h, buf, 64))
		return false;
	return lstrcmpiW(buf, name) == 0;
}

/* ---------- Vẽ ---------- */
class MemDC {
public:
	MemDC(HDC target, int w, int h) : m_target(target), m_w(w), m_h(h) {
		m_dc = CreateCompatibleDC(target);
		m_bmp = CreateCompatibleBitmap(target, w, h);
		m_old = SelectObject(m_dc, m_bmp);
	}
	~MemDC() {
		SelectObject(m_dc, m_old);
		DeleteObject(m_bmp);
		DeleteDC(m_dc);
	}
	HDC dc() const { return m_dc; }
	void flush(int x, int y) { BitBlt(m_target, x, y, m_w, m_h, m_dc, 0, 0, SRCCOPY); }
private:
	HDC m_target, m_dc;
	HBITMAP m_bmp;
	HGDIOBJ m_old;
	int m_w, m_h;
};

void fillRect(HDC dc, const RECT& rc, COLORREF c) {
	SetDCBrushColor(dc, c);
	FillRect(dc, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
}

inline Gdiplus::Color gcol(COLORREF c, BYTE a = 255) {
	return Gdiplus::Color(a, GetRValue(c), GetGValue(c), GetBValue(c));
}

void roundRectPath(Gdiplus::GraphicsPath& p, float x, float y, float w, float h, float r) {
	float d = r * 2.0f;
	if (d > w) d = w;
	if (d > h) d = h;
	if (d <= 0.0f) {
		p.AddRectangle(Gdiplus::RectF(x, y, w, h));
		return;
	}
	p.AddArc(x, y, d, d, 180.0f, 90.0f);
	p.AddArc(x + w - d, y, d, d, 270.0f, 90.0f);
	p.AddArc(x + w - d, y + h - d, d, d, 0.0f, 90.0f);
	p.AddArc(x, y + h - d, d, d, 90.0f, 90.0f);
	p.CloseFigure();
}

// Hình chữ nhật bo góc có khử răng cưa. fill/stroke = CLR_NONE nếu không dùng.
void drawRound(HDC dc, float x, float y, float w, float h, float r,
			   COLORREF fill, COLORREF stroke, float strokeW) {
	Gdiplus::Graphics g(dc);
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	float inset = (stroke != CLR_NONE) ? strokeW / 2.0f : 0.0f;
	float rr = r - inset;
	if (rr < 0.0f) rr = 0.0f;
	Gdiplus::GraphicsPath path;
	roundRectPath(path, x + inset, y + inset, w - 2.0f * inset, h - 2.0f * inset, rr);
	if (fill != CLR_NONE) {
		Gdiplus::SolidBrush b(gcol(fill));
		g.FillPath(&b, &path);
	}
	if (stroke != CLR_NONE) {
		Gdiplus::Pen pen(gcol(stroke), strokeW);
		g.DrawPath(&pen, &path);
	}
}

void drawEllipse(HDC dc, float x, float y, float w, float h, COLORREF fill, COLORREF stroke, float strokeW) {
	Gdiplus::Graphics g(dc);
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	float inset = (stroke != CLR_NONE) ? strokeW / 2.0f : 0.0f;
	if (fill != CLR_NONE) {
		Gdiplus::SolidBrush b(gcol(fill));
		g.FillEllipse(&b, x + inset, y + inset, w - 2.0f * inset, h - 2.0f * inset);
	}
	if (stroke != CLR_NONE) {
		Gdiplus::Pen pen(gcol(stroke), strokeW);
		g.DrawEllipse(&pen, x + inset, y + inset, w - 2.0f * inset, h - 2.0f * inset);
	}
}

void drawTick(HDC dc, float x, float y, float s, COLORREF color, float penW) {
	Gdiplus::Graphics g(dc);
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::Pen pen(gcol(color), penW);
	pen.SetLineJoin(Gdiplus::LineJoinRound);
	pen.SetStartCap(Gdiplus::LineCapRound);
	pen.SetEndCap(Gdiplus::LineCapRound);
	Gdiplus::PointF pts[3] = {
		Gdiplus::PointF(x + s * 0.24f, y + s * 0.53f),
		Gdiplus::PointF(x + s * 0.43f, y + s * 0.71f),
		Gdiplus::PointF(x + s * 0.77f, y + s * 0.31f)
	};
	g.DrawLines(&pen, pts, 3);
}

void drawDash(HDC dc, float x, float y, float s, COLORREF color, float penW) {
	Gdiplus::Graphics g(dc);
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::Pen pen(gcol(color), penW);
	pen.SetStartCap(Gdiplus::LineCapRound);
	pen.SetEndCap(Gdiplus::LineCapRound);
	g.DrawLine(&pen, x + s * 0.28f, y + s * 0.5f, x + s * 0.72f, y + s * 0.5f);
}

/* ---------- Metadata ---------- */
bool isPage(HWND h) { return GetPropW(h, kPropPage) != NULL; }

bool onCard(HWND h) {
	if (GetPropW(h, kPropCard) != NULL)
		return true;
	HWND parent = GetParent(h);
	return parent && isPage(parent);
}

COLORREF bgFor(HWND h) { return onCard(h) ? kCard : kBgMain; }

ButtonKind kindOf(HWND h) { return (ButtonKind)(INT_PTR)GetPropW(h, kPropKind); }

bool uiFocusVisible(HWND h) {
	LRESULT st = SendMessage(h, WM_QUERYUISTATE, 0, 0);
	return GetFocus() == h && !(st & UISF_HIDEFOCUS);
}

/* ---------- Nút bấm / checkbox / radio (NM_CUSTOMDRAW) ---------- */
void drawPushButton(NMCUSTOMDRAW* nm) {
	HWND h = nm->hdr.hwndFrom;
	RECT rc;
	GetClientRect(h, &rc);
	int w = rcW(rc), hh = rcH(rc);
	if (w <= 0 || hh <= 0)
		return;

	UINT st = nm->uItemState;
	bool disabled = (st & CDIS_DISABLED) != 0 || !IsWindowEnabled(h);
	bool pressed = (st & CDIS_SELECTED) != 0;
	bool hot = (st & CDIS_HOT) != 0;
	bool isDefault = (st & CDIS_DEFAULT) != 0;
	bool focus = uiFocusVisible(h);
	ButtonKind kind = kindOf(h);

	COLORREF fill, border, textColor;
	if (disabled) {
		fill = kBorder; border = kBorder; textColor = kTextDisabled;
	} else if (kind == ButtonKind::Primary) {
		fill = pressed ? kAccentPressed : (hot ? kAccentHover : kAccent);
		border = fill;
		textColor = RGB(0xFF, 0xFF, 0xFF);
	} else if (kind == ButtonKind::Danger) {
		fill = pressed ? kDangerSoftPressed : (hot ? kDangerSoft : kCard);
		border = kDanger;
		textColor = kDangerText;
	} else {
		fill = pressed ? kBorder : (hot ? RGB(0xF8, 0xFA, 0xFC) : kCard);
		border = (hot || isDefault) ? kAccent : kBorderInput;
		textColor = kTextPrimary;
	}

	MemDC mem(nm->hdc, w, hh);
	HDC dc = mem.dc();
	RECT local = { 0, 0, w, hh };
	fillRect(dc, local, bgFor(h));

	float inset = (float)px(3); // chừa chỗ cho vòng focus
	drawRound(dc, inset, inset, w - 2 * inset, hh - 2 * inset, (float)px(kButtonRadiusPx), fill, border, 1.0f);
	if (focus && !disabled)
		drawRound(dc, 0, 0, (float)w, (float)hh, (float)px(kButtonRadiusPx + 2), CLR_NONE, kAccent, (float)px(2));

	wchar_t text[160] = { 0 };
	GetWindowTextW(h, text, 160);
	HFONT font = controlFont(h);
	const wchar_t* glyph = (const wchar_t*)GetPropW(h, kPropGlyph);
	if (glyph && !g_hasIconFont)
		glyph = NULL;

	int tw = text[0] ? textWidth(dc, font, text) : 0;
	int gw = glyph ? px(16) : 0;
	int gap = (glyph && text[0]) ? px(6) : 0;
	int total = gw + gap + tw;
	int x0 = (w - total) / 2;
	if (x0 < px(8)) x0 = px(8);
	if (glyph) {
		RECT gr = { x0, 0, x0 + gw, hh };
		drawText(dc, glyph, gr, DT_CENTER | DT_VCENTER | DT_SINGLELINE, textColor, iconFont(px(14)));
	}
	if (text[0]) {
		RECT tr = { x0 + gw + gap, 0, w - px(8), hh };
		drawText(dc, text, tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, textColor, font);
	}
	mem.flush(rc.left, rc.top);
}

void drawCheckOrRadio(NMCUSTOMDRAW* nm, bool radio) {
	HWND h = nm->hdr.hwndFrom;
	RECT rc;
	GetClientRect(h, &rc);
	int w = rcW(rc), hh = rcH(rc);
	if (w <= 0 || hh <= 0)
		return;

	UINT st = nm->uItemState;
	bool disabled = (st & CDIS_DISABLED) != 0 || !IsWindowEnabled(h);
	bool hot = (st & CDIS_HOT) != 0;
	bool pressed = (st & CDIS_SELECTED) != 0;
	bool focus = uiFocusVisible(h);
	LRESULT check = SendMessage(h, BM_GETCHECK, 0, 0);
	bool checked = (check == BST_CHECKED);
	bool indeterminate = (check == BST_INDETERMINATE);

	MemDC mem(nm->hdc, w, hh);
	HDC dc = mem.dc();
	RECT local = { 0, 0, w, hh };
	fillRect(dc, local, bgFor(h));

	float s = (float)px(kCheckBoxPx);
	float bx = (float)px(kCheckPadLeftPx);
	float by = ((float)hh - s) / 2.0f;

	COLORREF boxFill, boxBorder;
	if (disabled) {
		boxFill = (checked || indeterminate) ? kBorderInput : kBgMain;
		boxBorder = kBorderInput;
	} else if (radio) {
		boxFill = (hot || pressed) ? kAccentSoft : kCard;
		boxBorder = (checked || hot || pressed) ? kAccent : kControlBorder;
	} else if (checked || indeterminate) {
		boxFill = pressed ? kAccentPressed : (hot ? kAccentHover : kAccent);
		boxBorder = boxFill;
	} else {
		boxFill = (hot || pressed) ? kAccentSoft : kCard;
		boxBorder = (hot || pressed) ? kAccent : kControlBorder;
	}

	if (radio) {
		drawEllipse(dc, bx, by, s, s, boxFill, boxBorder, checked ? (float)px(2) : 1.0f);
		if (checked) {
			float d = s * 0.5f;
			drawEllipse(dc, bx + (s - d) / 2.0f, by + (s - d) / 2.0f, d, d,
						disabled ? kBorderInput : kAccent, CLR_NONE, 0.0f);
		}
	} else {
		drawRound(dc, bx, by, s, s, (float)px(4), boxFill, boxBorder, 1.0f);
		if (checked)
			drawTick(dc, bx, by, s, RGB(0xFF, 0xFF, 0xFF), (float)px(2));
		else if (indeterminate)
			drawDash(dc, bx, by, s, RGB(0xFF, 0xFF, 0xFF), (float)px(2));
	}

	wchar_t text[256] = { 0 };
	GetWindowTextW(h, text, 256);
	if (text[0]) {
		RECT tr = { (int)(bx + s) + px(kCheckGapPx), 0, w, hh };
		drawText(dc, text, tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS,
				 disabled ? kTextDisabled : kTextPrimary, controlFont(h));
	}

	if (focus && !disabled)
		drawRound(dc, 1.0f, 1.0f, (float)(w - 2), (float)(hh - 2), (float)px(5), CLR_NONE, kAccent, (float)px(2));

	mem.flush(rc.left, rc.top);
}

LRESULT onButtonCustomDraw(NMCUSTOMDRAW* nm) {
	if (nm->dwDrawStage != CDDS_PREPAINT)
		return CDRF_DODEFAULT;
	HWND h = nm->hdr.hwndFrom;
	UINT type = (UINT)GetWindowLong(h, GWL_STYLE) & BS_TYPEMASK;
	switch (type) {
	case BS_PUSHBUTTON:
	case BS_DEFPUSHBUTTON:
		drawPushButton(nm);
		return CDRF_SKIPDEFAULT;
	case BS_CHECKBOX:
	case BS_AUTOCHECKBOX:
	case BS_3STATE:
	case BS_AUTO3STATE:
		drawCheckOrRadio(nm, false);
		return CDRF_SKIPDEFAULT;
	case BS_RADIOBUTTON:
	case BS_AUTORADIOBUTTON:
		drawCheckOrRadio(nm, true);
		return CDRF_SKIPDEFAULT;
	default:
		return CDRF_DODEFAULT; // group box: vẽ bằng subclass bên dưới
	}
}

/* ---------- Group box -> card (subclass) ---------- */
void paintCard(HWND h, HDC target) {
	RECT rc;
	GetClientRect(h, &rc);
	int w = rcW(rc), hh = rcH(rc);
	if (w <= 0 || hh <= 0)
		return;
	MemDC mem(target, w, hh);
	HDC dc = mem.dc();
	RECT local = { 0, 0, w, hh };
	fillRect(dc, local, bgFor(h));
	drawRound(dc, 0, 0, (float)w, (float)hh, (float)px(kCardRadiusPx), kCard, kBorder, 1.0f);

	wchar_t text[160] = { 0 };
	GetWindowTextW(h, text, 160);
	if (text[0]) {
		RECT tr = { px(12), px(8), w - px(12), px(8) + px(20) };
		drawText(dc, text, tr, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS,
				 kPrimary, semiboldOf(controlFont(h)));
	}
	mem.flush(0, 0);
}

LRESULT CALLBACK cardSubclassProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR) {
	switch (msg) {
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(h, &ps);
		paintCard(h, dc);
		EndPaint(h, &ps);
		return 0;
	}
	case WM_PRINTCLIENT:
		paintCard(h, (HDC)wp);
		return 0;
	case WM_NCDESTROY:
		RemoveWindowSubclass(h, cardSubclassProc, id);
		break;
	}
	return DefSubclassProc(h, msg, wp, lp);
}

/* ---------- Tab control (subclass) ---------- */
struct TabState {
	int hot;
	bool tracking;
};

void paintTab(HWND h, HDC target, const TabState* state) {
	RECT rc;
	GetClientRect(h, &rc);
	int w = rcW(rc), hh = rcH(rc);
	if (w <= 0 || hh <= 0)
		return;

	MemDC mem(target, w, hh);
	HDC dc = mem.dc();
	RECT local = { 0, 0, w, hh };
	fillRect(dc, local, kBgMain);

	int headerH = px(kTabHeaderPx);
	// thân tab = card
	drawRound(dc, 0, (float)headerH, (float)w, (float)(hh - headerH), (float)px(kCardRadiusPx), kCard, kBorder, 1.0f);

	HFONT font = controlFont(h);
	HFONT bold = semiboldOf(font);
	int count = TabCtrl_GetItemCount(h);
	int sel = TabCtrl_GetCurSel(h);
	bool focus = uiFocusVisible(h);

	for (int i = 0; i < count; i++) {
		RECT ir;
		if (!TabCtrl_GetItemRect(h, i, &ir))
			continue;
		RECT cell = { ir.left, 0, ir.right, headerH };

		wchar_t text[64] = { 0 };
		TCITEMW item;
		ZeroMemory(&item, sizeof(item));
		item.mask = TCIF_TEXT;
		item.pszText = text;
		item.cchTextMax = 64;
		TabCtrl_GetItem(h, i, &item);

		bool isSel = (i == sel);
		bool isHot = (state && state->hot == i);
		if (isHot && !isSel)
			drawRound(dc, (float)(cell.left + px(4)), (float)px(5), (float)(rcW(cell) - px(8)),
					  (float)(headerH - px(5) - px(6)), (float)px(6), kBorder, CLR_NONE, 0.0f);

		drawText(dc, text, cell, DT_CENTER | DT_VCENTER | DT_SINGLELINE,
				 isSel ? kAccent : (isHot ? kTextPrimary : kTextMuted), isSel ? bold : font);

		if (isSel) {
			// thanh gạch chân đè lên viền trên của card
			float bh = (float)px(3);
			drawRound(dc, (float)(cell.left + px(12)), (float)headerH - bh, (float)(rcW(cell) - px(24)), bh,
					  bh / 2.0f, kAccent, CLR_NONE, 0.0f);
			if (focus)
				drawRound(dc, (float)(cell.left + px(2)), (float)px(3), (float)(rcW(cell) - px(4)),
						  (float)(headerH - px(8)), (float)px(6), CLR_NONE, kAccent, (float)px(2));
		}
	}
	mem.flush(0, 0);
}

LRESULT CALLBACK tabSubclassProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ref) {
	TabState* state = (TabState*)ref;
	switch (msg) {
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(h, &ps);
		paintTab(h, dc, state);
		EndPaint(h, &ps);
		return 0;
	}
	case WM_PRINTCLIENT:
		paintTab(h, (HDC)wp, state);
		return 0;
	case WM_MOUSEMOVE: {
		TCHITTESTINFO ht;
		ZeroMemory(&ht, sizeof(ht));
		ht.pt.x = GET_X_LPARAM(lp);
		ht.pt.y = GET_Y_LPARAM(lp);
		int idx = TabCtrl_HitTest(h, &ht);
		if (idx != state->hot) {
			state->hot = idx;
			InvalidateRect(h, NULL, FALSE);
		}
		if (!state->tracking) {
			TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, h, 0 };
			if (TrackMouseEvent(&tme))
				state->tracking = true;
		}
		break;
	}
	case WM_MOUSELEAVE:
		state->hot = -1;
		state->tracking = false;
		InvalidateRect(h, NULL, FALSE);
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		InvalidateRect(h, NULL, FALSE);
		break;
	case WM_NCDESTROY:
		RemoveWindowSubclass(h, tabSubclassProc, id);
		delete state;
		break;
	}
	return DefSubclassProc(h, msg, wp, lp);
}

/* ---------- Áp dụng cho dialog ---------- */
void applyTitleBar(HWND h) {
	typedef HRESULT (WINAPI *DwmSetWindowAttributeFn)(HWND, DWORD, LPCVOID, DWORD);
	static DwmSetWindowAttributeFn fn = NULL;
	static bool resolved = false;
	if (!resolved) {
		resolved = true;
		wchar_t path[MAX_PATH];
		UINT n = GetSystemDirectoryW(path, MAX_PATH);
		if (n > 0 && n < MAX_PATH - 16) {
			lstrcatW(path, L"\\dwmapi.dll");
			HMODULE m = LoadLibraryW(path);
			if (m)
				fn = (DwmSetWindowAttributeFn)GetProcAddress(m, "DwmSetWindowAttribute");
		}
	}
	if (!fn)
		return;
	// Chỉ có tác dụng trên Windows 11 (build 22000+); các bản cũ bỏ qua lỗi và giữ thanh tiêu đề mặc định.
	COLORREF caption = kSidebar, text = kTextOnDark;
	fn(h, 35 /*DWMWA_CAPTION_COLOR*/, &caption, sizeof(caption));
	fn(h, 36 /*DWMWA_TEXT_COLOR*/, &text, sizeof(text));
}

struct Child {
	HWND h;
	RECT r; // tọa độ client của parent
};

// Control dạng "nhãn" (static 1 dòng, checkbox, radio) -> true. isButton cho biết loại.
bool isLabelLike(HWND h, bool& isButton) {
	if (isClass(h, L"Static")) {
		UINT t = (UINT)GetWindowLong(h, GWL_STYLE) & SS_TYPEMASK;
		isButton = false;
		return t == SS_LEFT || t == SS_SIMPLE || t == SS_LEFTNOWORDWRAP;
	}
	if (isClass(h, L"Button")) {
		UINT t = (UINT)GetWindowLong(h, GWL_STYLE) & BS_TYPEMASK;
		isButton = true;
		return t == BS_CHECKBOX || t == BS_AUTOCHECKBOX || t == BS_RADIOBUTTON ||
			   t == BS_AUTORADIOBUTTON || t == BS_3STATE || t == BS_AUTO3STATE;
	}
	return false;
}

// Font/DPI khác nhau có thể làm chữ rộng hơn control đã khai báo trong .rc:
// nới control ra (không bao giờ thu nhỏ) tới khi chạm control bên phải hoặc mép dialog.
void fitLabel(const vector<Child>& all, const Child& c, int parentW) {
	bool isButton = false;
	if (!isLabelLike(c.h, isButton))
		return;
	wchar_t text[256] = { 0 };
	if (!GetWindowTextW(c.h, text, 256) || !text[0])
		return;

	HFONT font = controlFont(c.h);
	HDC dc = GetDC(c.h);
	HFONT old = (HFONT)SelectObject(dc, font);
	TEXTMETRICW tm;
	GetTextMetricsW(dc, &tm);
	SIZE sz = { 0, 0 };
	GetTextExtentPoint32W(dc, text, lstrlenW(text), &sz);
	SelectObject(dc, old);
	ReleaseDC(c.h, dc);

	int curW = rcW(c.r), curH = rcH(c.r);
	if (!isButton && curH >= tm.tmHeight * 17 / 10)
		return; // static nhiều dòng: giữ nguyên
	int need = isButton ? px(kCheckPadLeftPx + kCheckBoxPx + kCheckGapPx) + sz.cx + px(6) : sz.cx + px(2);
	if (need <= curW)
		return;

	int limit = parentW - px(8);
	for (size_t i = 0; i < all.size(); i++) {
		const Child& o = all[i];
		if (o.h == c.h) continue;
		if (o.r.top >= c.r.bottom || o.r.bottom <= c.r.top) continue; // khác hàng
		if (o.r.left <= c.r.left) continue;                           // nằm bên trái
		int l = o.r.left - px(4);
		if (l < limit) limit = l;
	}
	int maxW = limit - c.r.left;
	int newW = need < maxW ? need : maxW;
	if (newW > curW)
		SetWindowPos(c.h, NULL, 0, 0, newW, curH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void styleListView(HWND lv) {
	SetWindowTheme(lv, L"Explorer", NULL);
	ListView_SetExtendedListViewStyleEx(lv, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);
	ListView_SetBkColor(lv, kCard);
	ListView_SetTextBkColor(lv, kCard);
	ListView_SetTextColor(lv, kTextPrimary);
}

void prepareChildren(HWND parent, bool allOnCard) {
	vector<Child> children;
	for (HWND c = GetWindow(parent, GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT)) {
		Child item;
		item.h = c;
		GetWindowRect(c, &item.r);
		MapWindowPoints(NULL, parent, (POINT*)&item.r, 2);
		children.push_back(item);
	}

	// 1) group box -> card
	vector<RECT> cards;
	for (size_t i = 0; i < children.size(); i++) {
		HWND c = children[i].h;
		if (!isClass(c, L"Button"))
			continue;
		if (((UINT)GetWindowLong(c, GWL_STYLE) & BS_TYPEMASK) != BS_GROUPBOX)
			continue;
		SetWindowLong(c, GWL_STYLE, GetWindowLong(c, GWL_STYLE) | WS_CLIPSIBLINGS);
		SetWindowSubclass(c, cardSubclassProc, 1, 0);
		cards.push_back(children[i].r);
	}

	// 2) đánh dấu control nằm trên card (để chọn màu nền đúng) + các tinh chỉnh theo loại
	RECT pc;
	GetClientRect(parent, &pc);
	for (size_t i = 0; i < children.size(); i++) {
		HWND c = children[i].h;
		bool inCard = allOnCard;
		if (!inCard) {
			POINT center = { (children[i].r.left + children[i].r.right) / 2,
							 (children[i].r.top + children[i].r.bottom) / 2 };
			for (size_t k = 0; k < cards.size(); k++)
				if (PtInRect(&cards[k], center)) { inCard = true; break; }
		}
		if (inCard)
			SetPropW(c, kPropCard, (HANDLE)1);

		if (isClass(c, L"SysListView32"))
			styleListView(c);
	}

	// 3) nới nhãn bị thiếu chỗ
	for (size_t i = 0; i < children.size(); i++)
		fitLabel(children, children[i], rcW(pc));
}

} // namespace

/* =================== API công khai =================== */

int dpi(int px96) {
	ensureInit();
	return px(px96);
}

bool isEnabled() {
	if (g_enabledCache < 0) {
		bool on = IsThemeActive() != FALSE;
		if (on) {
			HIGHCONTRASTW hc;
			ZeroMemory(&hc, sizeof(hc));
			hc.cbSize = sizeof(hc);
			if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) && (hc.dwFlags & HCF_HIGHCONTRASTON))
				on = false;
		}
		g_enabledCache = on ? 1 : 0;
	}
	return g_enabledCache == 1;
}

void apply(HWND hDlg) {
	if (!isEnabled())
		return;
	ensureInit();
	applyTitleBar(hDlg);
	prepareChildren(hDlg, false);
}

void markPage(HWND hPage) {
	SetPropW(hPage, kPropPage, (HANDLE)1);
}

void applyPage(HWND hPage) {
	if (!isEnabled())
		return;
	ensureInit();
	markPage(hPage);
	prepareChildren(hPage, true);
}

void setButtonKind(HWND hButton, ButtonKind kind, const wchar_t* glyph) {
	if (!hButton)
		return;
	SetPropW(hButton, kPropKind, (HANDLE)(INT_PTR)kind);
	SetPropW(hButton, kPropGlyph, (HANDLE)glyph); // glyph phải là chuỗi literal (tồn tại suốt đời process)
	InvalidateRect(hButton, NULL, FALSE);
}

void setLabelRole(HWND hLabel, LabelRole role) {
	if (!hLabel)
		return;
	SetPropW(hLabel, kPropRole, (HANDLE)(INT_PTR)role);
	InvalidateRect(hLabel, NULL, TRUE);
}

int styleTabControl(HWND hTab) {
	if (!isEnabled())
		return 0;
	ensureInit();

	SetWindowLong(hTab, GWL_STYLE, GetWindowLong(hTab, GWL_STYLE) | WS_CLIPSIBLINGS);

	// Chiều rộng mỗi tab = nhãn dài nhất (semibold) + đệm 2 bên.
	HDC dc = GetDC(hTab);
	HFONT bold = semiboldOf(controlFont(hTab));
	int maxW = 0;
	int count = TabCtrl_GetItemCount(hTab);
	for (int i = 0; i < count; i++) {
		wchar_t text[64] = { 0 };
		TCITEMW item;
		ZeroMemory(&item, sizeof(item));
		item.mask = TCIF_TEXT;
		item.pszText = text;
		item.cchTextMax = 64;
		if (TabCtrl_GetItem(hTab, i, &item)) {
			int tw = textWidth(dc, bold, text);
			if (tw > maxW) maxW = tw;
		}
	}
	ReleaseDC(hTab, dc);

	int headerH = px(kTabHeaderPx);
	TabCtrl_SetItemSize(hTab, maxW + px(36), headerH);

	TabState* state = new TabState();
	state->hot = -1;
	state->tracking = false;
	SetWindowSubclass(hTab, tabSubclassProc, 1, (DWORD_PTR)state);
	return headerH;
}

RECT tabPageRect(HWND hTab, HWND hParent) {
	ensureInit();
	RECT rc;
	GetWindowRect(hTab, &rc);
	MapWindowPoints(NULL, hParent, (POINT*)&rc, 2);
	int inset = px(kTabInsetPx);
	rc.left += inset;
	rc.right -= inset;
	rc.top += px(kTabHeaderPx) + inset;
	rc.bottom -= inset;
	return rc;
}

void paintHeader(HDC hdc, const RECT& rc, HICON icon, const wchar_t* title,
				 const wchar_t* subtitle, const wchar_t* status, bool statusOn, HFONT baseFont) {
	int w = rcW(rc), hh = rcH(rc);
	if (w <= 0 || hh <= 0)
		return;

	if (!isEnabled()) {
		// Fallback: màu hệ thống (High Contrast / không có visual styles)
		FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
		RECT tr = { rc.left + 8, rc.top, rc.right - 8, rc.bottom };
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
		HFONT old = (HFONT)SelectObject(hdc, baseFont ? baseFont : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
		wchar_t line[256];
		wsprintfW(line, L"%s   %s", title, status);
		DrawTextW(hdc, line, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
		SelectObject(hdc, old);
		return;
	}

	ensureInit();
	MemDC mem(hdc, w, hh);
	HDC dc = mem.dc();
	RECT local = { 0, 0, w, hh };
	fillRect(dc, local, kSidebar);
	RECT line = { 0, hh - 1, w, hh };
	fillRect(dc, line, RGB(0x1E, 0x29, 0x3B));

	int margin = px(14);
	int x = margin;

	int iconSize = px(24);
	if (icon) {
		DrawIconEx(dc, x, (hh - iconSize) / 2, icon, iconSize, iconSize, 0, NULL, DI_NORMAL);
		x += iconSize + px(10);
	}

	HFONT titleFont = uiFont(12, true);
	HFONT subFont = baseFont ? baseFont : uiFont(9, false);
	int tw = textWidth(dc, titleFont, title);
	RECT tr = { x, 0, x + tw + px(2), hh - 1 };
	drawText(dc, title, tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE, kTextOnDark, titleFont);
	x += tw + px(10);

	// pill trạng thái ở bên phải
	int pillH = px(22);
	int pillTextW = textWidth(dc, subFont, status);
	int dot = px(8);
	int pillW = px(10) + dot + px(6) + pillTextW + px(12);
	int pillX = w - margin - pillW;
	int pillY = (hh - 1 - pillH) / 2;

	if (subtitle && subtitle[0]) {
		int maxSubW = pillX - px(10) - x;
		if (maxSubW > px(20)) {
			RECT sr = { x, 0, x + maxSubW, hh - 1 };
			drawText(dc, subtitle, sr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS,
					 kTextOnDarkMuted, subFont);
		}
	}

	drawRound(dc, (float)pillX, (float)pillY, (float)pillW, (float)pillH, pillH / 2.0f,
			  RGB(0x1E, 0x29, 0x3B), CLR_NONE, 0.0f);
	drawEllipse(dc, (float)(pillX + px(10)), pillY + (pillH - dot) / 2.0f, (float)dot, (float)dot,
				statusOn ? kSuccess : kTextOnDarkMuted, CLR_NONE, 0.0f);
	RECT pr = { pillX + px(10) + dot + px(6), pillY, pillX + pillW - px(8), pillY + pillH };
	drawText(dc, status, pr, DT_LEFT | DT_VCENTER | DT_SINGLELINE, kTextOnDark, subFont);

	mem.flush(rc.left, rc.top);
}

bool handleMessage(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp, INT_PTR& result) {
	switch (msg) {
	case WM_THEMECHANGED:
	case WM_SYSCOLORCHANGE:
	case WM_SETTINGCHANGE:
		g_enabledCache = -1;
		return false;
	}
	if (!isEnabled())
		return false;
	ensureInit();

	switch (msg) {
	case WM_CTLCOLORDLG:
		result = (INT_PTR)(isPage(hDlg) ? g_brCard : g_brBg);
		return true;

	case WM_ERASEBKGND: {
		RECT rc;
		GetClientRect(hDlg, &rc);
		FillRect((HDC)wp, &rc, isPage(hDlg) ? g_brCard : g_brBg);
		result = TRUE;
		return true;
	}

	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN: {
		HDC dc = (HDC)wp;
		HWND c = (HWND)lp;
		bool disabled = !IsWindowEnabled(c);
		if (isClass(c, L"Edit")) {
			// edit chỉ đọc/vô hiệu hóa gửi WM_CTLCOLORSTATIC
			SetTextColor(dc, disabled ? kTextDisabled : kTextPrimary);
			SetBkColor(dc, kCard);
			result = (INT_PTR)g_brCard;
			return true;
		}
		bool card = onCard(c);
		LabelRole role = (LabelRole)(INT_PTR)GetPropW(c, kPropRole);
		COLORREF color = kTextPrimary;
		if (role == LabelRole::Title)
			color = kPrimary;
		else if (role == LabelRole::Muted)
			color = card ? kTextSecondary : kTextMuted;
		if (disabled)
			color = kTextDisabled;
		SetTextColor(dc, color);
		SetBkColor(dc, card ? kCard : kBgMain);
		SetBkMode(dc, TRANSPARENT);
		result = (INT_PTR)(card ? g_brCard : g_brBg);
		return true;
	}

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX: {
		HDC dc = (HDC)wp;
		SetTextColor(dc, kTextPrimary);
		SetBkColor(dc, kCard);
		result = (INT_PTR)g_brCard;
		return true;
	}

	case WM_NOTIFY: {
		NMHDR* nh = (NMHDR*)lp;
		if (nh && nh->code == NM_CUSTOMDRAW && isClass(nh->hwndFrom, L"Button")) {
			LRESULT r = onButtonCustomDraw((NMCUSTOMDRAW*)lp);
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)r);
			result = TRUE;
			return true;
		}
		return false;
	}
	}
	return false;
}

}

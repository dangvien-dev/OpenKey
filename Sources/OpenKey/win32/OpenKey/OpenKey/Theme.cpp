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

/* ---------- Token màu: định nghĩa (giá trị ban đầu là palette sáng; applyPalette đổi theo chế độ) ---------- */
COLORREF kBgMain        = RGB(0xF3, 0xF3, 0xF3);
COLORREF kCard          = RGB(0xFF, 0xFF, 0xFF);
COLORREF kBorder        = RGB(0xE5, 0xE5, 0xE5);
COLORREF kBorderInput   = RGB(0xD1, 0xD1, 0xD1);
COLORREF kControlBorder = RGB(0x70, 0x70, 0x70);
COLORREF kTextPrimary   = RGB(0x1A, 0x1A, 0x1A);
COLORREF kTextSecondary = RGB(0x61, 0x61, 0x61);
COLORREF kTextMuted     = RGB(0x5C, 0x5C, 0x5C);
COLORREF kTextDisabled  = RGB(0xA6, 0xA6, 0xA6);
COLORREF kAccent        = RGB(0x0F, 0x6C, 0xBD);
COLORREF kAccentHover   = RGB(0x11, 0x5E, 0xA3);
COLORREF kAccentPressed = RGB(0x0F, 0x54, 0x8C);
COLORREF kAccentSoft    = RGB(0xEB, 0xF3, 0xFC);
COLORREF kOnAccent      = RGB(0xFF, 0xFF, 0xFF);
COLORREF kHover         = RGB(0xF5, 0xF5, 0xF5);
COLORREF kDanger        = RGB(0xC4, 0x2B, 0x1C);
COLORREF kDangerText    = RGB(0xC4, 0x2B, 0x1C);
COLORREF kDangerSoft    = RGB(0xFD, 0xE7, 0xE9);
COLORREF kDangerSoftPressed = RGB(0xF9, 0xD2, 0xD5);

namespace {

/* ---------- Palette sáng / tối ---------- */
struct Palette {
	COLORREF bgMain, card, border, borderInput, controlBorder;
	COLORREF textPrimary, textSecondary, textMuted, textDisabled;
	COLORREF accent, accentHover, accentPressed, accentSoft, onAccent, hover;
	COLORREF danger, dangerText, dangerSoft, dangerSoftPressed;
};

const Palette kPaletteLight = {
	RGB(0xF3, 0xF3, 0xF3), RGB(0xFF, 0xFF, 0xFF), RGB(0xE5, 0xE5, 0xE5), RGB(0xD1, 0xD1, 0xD1), RGB(0x70, 0x70, 0x70),
	RGB(0x1A, 0x1A, 0x1A), RGB(0x61, 0x61, 0x61), RGB(0x5C, 0x5C, 0x5C), RGB(0xA6, 0xA6, 0xA6),
	RGB(0x0F, 0x6C, 0xBD), RGB(0x11, 0x5E, 0xA3), RGB(0x0F, 0x54, 0x8C), RGB(0xEB, 0xF3, 0xFC), RGB(0xFF, 0xFF, 0xFF), RGB(0xF5, 0xF5, 0xF5),
	RGB(0xC4, 0x2B, 0x1C), RGB(0xC4, 0x2B, 0x1C), RGB(0xFD, 0xE7, 0xE9), RGB(0xF9, 0xD2, 0xD5)
};

// Kiểu Windows 11 tối: nền #202020, card #2B2B2B, màu nhấn xanh sáng hơn và chữ đặt trên nút chính là chữ TỐI
// (đã kiểm tra độ tương phản: chữ trên card 13:1, chữ trên nút chính 6:1, viền ô nhập 3:1).
const Palette kPaletteDark = {
	RGB(0x20, 0x20, 0x20), RGB(0x2B, 0x2B, 0x2B), RGB(0x3D, 0x3D, 0x3D), RGB(0x75, 0x75, 0x75), RGB(0xA6, 0xA6, 0xA6),
	RGB(0xF5, 0xF5, 0xF5), RGB(0xC7, 0xC7, 0xC7), RGB(0xBD, 0xBD, 0xBD), RGB(0x7A, 0x7A, 0x7A),
	RGB(0x4C, 0x9B, 0xEF), RGB(0x6A, 0xAE, 0xF3), RGB(0x35, 0x84, 0xDA), RGB(0x26, 0x34, 0x45), RGB(0x0B, 0x1A, 0x2B), RGB(0x35, 0x35, 0x35),
	RGB(0xFF, 0x7B, 0x72), RGB(0xFF, 0x7B, 0x72), RGB(0x3C, 0x25, 0x28), RGB(0x4D, 0x2D, 0x31)
};

/* ---------- Window properties dùng để gắn metadata lên control ---------- */
const wchar_t* const kPropKind  = L"OKTheme.Kind";   // ButtonKind
const wchar_t* const kPropGlyph = L"OKTheme.Glyph";  // const wchar_t* (literal)
const wchar_t* const kPropRole  = L"OKTheme.Role";   // LabelRole
const wchar_t* const kPropCard  = L"OKTheme.Card";   // control nằm trên card trắng
const wchar_t* const kPropPage  = L"OKTheme.Page";   // dialog là trang của tab (nền trắng)

/* ---------- Kích thước @96dpi ---------- */
const int kTabHeaderPx   = 32;
const int kTabInsetPx    = 8;
const int kCardRadiusPx  = 6;
const int kButtonRadiusPx = 4;
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
int       g_mode = 0;          // 0 = theo Windows, 1 = sáng, 2 = tối
int       g_darkCache = -1;    // -1 = chưa tính; 0/1 = kết quả isDark()
int       g_appliedDark = -1;  // bảng màu đang áp dụng

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

	g_hasIconFont = fontInstalled(L"Segoe MDL2 Assets");
}

// Giá trị AppsUseLightTheme (0 = ứng dụng dùng giao diện tối). Không đọc được thì coi như sáng.
bool windowsAppsUseDark() {
	HKEY key;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &key) != ERROR_SUCCESS)
		return false;
	DWORD value = 1, size = sizeof(value), type = 0;
	LONG rc = RegQueryValueExW(key, L"AppsUseLightTheme", NULL, &type, (LPBYTE)&value, &size);
	RegCloseKey(key);
	return rc == ERROR_SUCCESS && type == REG_DWORD && value == 0;
}

bool computeDark() {
	if (g_mode == 1) return false;
	if (g_mode == 2) return true;
	return windowsAppsUseDark();
}

// Gán token theo palette và dựng lại 2 brush nền. Trả về true nếu bảng màu thật sự đổi.
bool applyPalette(bool dark) {
	if ((dark ? 1 : 0) == g_appliedDark && g_brBg && g_brCard)
		return false;
	const Palette& p = dark ? kPaletteDark : kPaletteLight;
	kBgMain = p.bgMain; kCard = p.card; kBorder = p.border; kBorderInput = p.borderInput; kControlBorder = p.controlBorder;
	kTextPrimary = p.textPrimary; kTextSecondary = p.textSecondary; kTextMuted = p.textMuted; kTextDisabled = p.textDisabled;
	kAccent = p.accent; kAccentHover = p.accentHover; kAccentPressed = p.accentPressed; kAccentSoft = p.accentSoft;
	kOnAccent = p.onAccent; kHover = p.hover;
	kDanger = p.danger; kDangerText = p.dangerText; kDangerSoft = p.dangerSoft; kDangerSoftPressed = p.dangerSoftPressed;
	if (g_brBg) DeleteObject(g_brBg);
	if (g_brCard) DeleteObject(g_brCard);
	g_brBg = CreateSolidBrush(kBgMain);
	g_brCard = CreateSolidBrush(kCard);
	g_appliedDark = dark ? 1 : 0;
	return true;
}

// Bảo đảm bảng màu khớp với chế độ hiện tại. Gọi đầu mỗi lần vẽ/áp dụng; rẻ khi không có gì đổi (đã cache).
void ensureTheme() {
	if (g_darkCache < 0)
		g_darkCache = computeDark() ? 1 : 0;
	applyPalette(g_darkCache == 1);
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
		textColor = kOnAccent;
	} else if (kind == ButtonKind::Danger) {
		fill = pressed ? kDangerSoftPressed : (hot ? kDangerSoft : kCard);
		border = (hot || pressed) ? kDanger : kBorderInput;
		textColor = kDangerText;
	} else {
		fill = pressed ? kBorder : (hot ? kHover : kCard);
		border = isDefault ? kAccent : kBorderInput;
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
		drawRound(dc, bx, by, s, s, (float)px(3), boxFill, boxBorder, 1.0f);
		if (checked)
			drawTick(dc, bx, by, s, kOnAccent, (float)px(2));
		else if (indeterminate)
			drawDash(dc, bx, by, s, kOnAccent, (float)px(2));
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
				 kTextPrimary, semiboldOf(controlFont(h)));
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
				 (isSel || isHot) ? kTextPrimary : kTextMuted, isSel ? bold : font);

		if (isSel) {
			// thanh gạch chân đè lên viền trên của card
			float bh = (float)px(2);
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

/* ---------- Ô nhập bo góc: Edit và ComboBox (subclass) ---------- */
const int kInputRadiusPx = 6;
const int kInputPadXPx   = 8;

struct InputState {
	bool hot;
	bool tracking;
};

COLORREF inputBorder(bool disabled, bool focus, bool hot) {
	if (disabled) return kBorder;
	if (focus) return kAccent;
	return hot ? kControlBorder : kBorderInput;
}

// Khung bo góc + nền; vùng ngoài góc bo được tô bằng màu nền của cha để không lộ góc vuông.
void paintInputBox(HDC dc, int w, int hh, HWND h, bool disabled, bool focus, bool hot) {
	RECT local = { 0, 0, w, hh };
	fillRect(dc, local, bgFor(h));
	float sw = focus ? (float)px(2) : 1.0f;
	drawRound(dc, 0, 0, (float)w, (float)hh, (float)px(kInputRadiusPx),
			  disabled ? kBgMain : kCard, inputBorder(disabled, focus, hot), sw);
}

void trackHot(HWND h, InputState* st, bool frameOnly) {
	if (!st->hot) {
		st->hot = true;
		if (frameOnly) RedrawWindow(h, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		else InvalidateRect(h, NULL, FALSE);
	}
	if (!st->tracking) {
		TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, h, 0 };
		if (TrackMouseEvent(&tme))
			st->tracking = true;
	}
}

void leaveHot(HWND h, InputState* st, bool frameOnly) {
	st->hot = false;
	st->tracking = false;
	if (frameOnly) RedrawWindow(h, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
	else InvalidateRect(h, NULL, FALSE);
}

// ---- Edit: vẽ khung ở vùng non-client, vùng client thụt vào để chữ không đè lên góc bo
void paintEditFrame(HWND h, const InputState* st) {
	RECT wr;
	GetWindowRect(h, &wr);
	int w = rcW(wr), hh = rcH(wr);
	if (w <= 0 || hh <= 0)
		return;
	HDC dc = GetWindowDC(h);
	if (!dc)
		return;
	RECT cr;
	GetClientRect(h, &cr);
	POINT org = { 0, 0 };
	ClientToScreen(h, &org);
	int cx = org.x - wr.left, cy = org.y - wr.top;
	ExcludeClipRect(dc, cx, cy, cx + rcW(cr), cy + rcH(cr)); // không đè lên vùng chữ
	{
		MemDC mem(dc, w, hh);
		paintInputBox(mem.dc(), w, hh, h, !IsWindowEnabled(h), GetFocus() == h, st && st->hot);
		mem.flush(0, 0);
	}
	ReleaseDC(h, dc);
}

LRESULT CALLBACK editSubclassProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ref) {
	InputState* st = (InputState*)ref;
	switch (msg) {
	case WM_NCCALCSIZE: {
		LRESULT r = DefSubclassProc(h, msg, wp, lp);
		RECT* rc = wp ? &((NCCALCSIZE_PARAMS*)lp)->rgrc[0] : (RECT*)lp;
		int padX = px(kInputPadXPx);
		int padY = px(4);
		if (!(GetWindowLong(h, GWL_STYLE) & ES_MULTILINE)) {
			// một dòng: căn giữa chữ theo chiều dọc
			HDC dc = GetDC(h);
			HFONT old = (HFONT)SelectObject(dc, controlFont(h));
			TEXTMETRICW tm;
			GetTextMetricsW(dc, &tm);
			SelectObject(dc, old);
			ReleaseDC(h, dc);
			padY = ((rc->bottom - rc->top) - tm.tmHeight) / 2;
			if (padY < px(2)) padY = px(2);
		}
		rc->left += padX;
		rc->right -= padX;
		rc->top += padY;
		rc->bottom -= padY;
		if (rc->right < rc->left) rc->right = rc->left;
		if (rc->bottom < rc->top) rc->bottom = rc->top;
		return r;
	}
	case WM_NCPAINT:
		paintEditFrame(h, st);
		return 0;
	case WM_MOUSEMOVE:
		trackHot(h, st, true);
		break;
	case WM_MOUSELEAVE:
		leaveHot(h, st, true);
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_ENABLE: {
		LRESULT r = DefSubclassProc(h, msg, wp, lp);
		RedrawWindow(h, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		return r;
	}
	case WM_NCDESTROY:
		RemoveWindowSubclass(h, editSubclassProc, id);
		delete st;
		break;
	}
	return DefSubclassProc(h, msg, wp, lp);
}

void styleEdit(HWND h) {
	LONG style = GetWindowLong(h, GWL_STYLE);
	if (style & (WS_VSCROLL | WS_HSCROLL))
		return; // có thanh cuộn: giữ nguyên
	DWORD_PTR ref = 0;
	if (GetWindowSubclass(h, editSubclassProc, 1, &ref))
		return; // đã áp dụng
	SetWindowLong(h, GWL_STYLE, style & ~WS_BORDER);
	SetWindowLong(h, GWL_EXSTYLE, GetWindowLong(h, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);
	SetWindowSubclass(h, editSubclassProc, 1, (DWORD_PTR)new InputState{ false, false });
	SetWindowPos(h, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

// ---- ComboBox kiểu danh sách thả xuống: tự vẽ toàn bộ (khung bo góc, chữ mục đang chọn, mũi tên)
void paintCombo(HWND h, HDC target, const InputState* st) {
	RECT rc;
	GetClientRect(h, &rc);
	int w = rcW(rc), hh = rcH(rc);
	if (w <= 0 || hh <= 0)
		return;
	bool disabled = !IsWindowEnabled(h);
	bool dropped = SendMessage(h, CB_GETDROPPEDSTATE, 0, 0) != 0;
	MemDC mem(target, w, hh);
	HDC dc = mem.dc();
	paintInputBox(dc, w, hh, h, disabled, GetFocus() == h || dropped, st && st->hot);

	wchar_t text[256] = { 0 };
	int sel = (int)SendMessage(h, CB_GETCURSEL, 0, 0);
	if (sel != CB_ERR) {
		int len = (int)SendMessage(h, CB_GETLBTEXTLEN, sel, 0);
		if (len > 0 && len < 255)
			SendMessage(h, CB_GETLBTEXT, sel, (LPARAM)text);
	}
	RECT tr = { px(kInputPadXPx + 2), 0, w - px(28), hh };
	drawText(dc, text, tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS,
			 disabled ? kTextDisabled : kTextPrimary, controlFont(h));

	// mũi tên xổ xuống
	{
		float cx = (float)(w - px(15)), cy = hh / 2.0f;
		float a = (float)px(4), b = (float)px(2);
		Gdiplus::Graphics g(dc);
		g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		Gdiplus::Pen pen(gcol(disabled ? kTextDisabled : kTextSecondary), (float)px(2) * 0.8f);
		pen.SetStartCap(Gdiplus::LineCapRound);
		pen.SetEndCap(Gdiplus::LineCapRound);
		pen.SetLineJoin(Gdiplus::LineJoinRound);
		Gdiplus::PointF pts[3] = { Gdiplus::PointF(cx - a, cy - b), Gdiplus::PointF(cx, cy + b), Gdiplus::PointF(cx + a, cy - b) };
		g.DrawLines(&pen, pts, 3);
	}
	mem.flush(rc.left, rc.top);
}

LRESULT CALLBACK comboSubclassProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ref) {
	InputState* st = (InputState*)ref;
	switch (msg) {
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(h, &ps);
		paintCombo(h, dc, st);
		EndPaint(h, &ps);
		return 0;
	}
	case WM_PRINTCLIENT:
		paintCombo(h, (HDC)wp, st);
		return 0;
	case WM_MOUSEMOVE:
		trackHot(h, st, false);
		break;
	case WM_MOUSELEAVE:
		leaveHot(h, st, false);
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_ENABLE: {
		LRESULT r = DefSubclassProc(h, msg, wp, lp);
		InvalidateRect(h, NULL, FALSE);
		return r;
	}
	case WM_NCDESTROY:
		RemoveWindowSubclass(h, comboSubclassProc, id);
		delete st;
		break;
	}
	return DefSubclassProc(h, msg, wp, lp);
}

// Danh sách thả xuống của combobox là cửa sổ riêng: đặt theme để thanh cuộn khớp chế độ sáng/tối.
void themeComboList(HWND h) {
	COMBOBOXINFO cbi;
	ZeroMemory(&cbi, sizeof(cbi));
	cbi.cbSize = sizeof(cbi);
	if (GetComboBoxInfo(h, &cbi) && cbi.hwndList)
		SetWindowTheme(cbi.hwndList, isDark() ? L"DarkMode_Explorer" : L"Explorer", NULL);
}

void styleCombo(HWND h) {
	if (((UINT)GetWindowLong(h, GWL_STYLE) & 0x3) != CBS_DROPDOWNLIST)
		return; // chỉ kiểu danh sách thả xuống (không có ô nhập)
	DWORD_PTR ref = 0;
	if (!GetWindowSubclass(h, comboSubclassProc, 1, &ref)) {
		SetWindowSubclass(h, comboSubclassProc, 1, (DWORD_PTR)new InputState{ false, false });
		InvalidateRect(h, NULL, TRUE);
	}
	themeComboList(h);
}

/* ---------- SysLink: vẽ liên kết bằng màu nhấn (đọc được cả ở nền tối) ---------- */
// Giữ nguyên hành vi của control (click, bàn phím, thông báo NM_CLICK): chỉ thay cách vẽ.
wstring linkPlainText(HWND h) {
	wchar_t buf[512] = { 0 };
	GetWindowTextW(h, buf, 512);
	wstring in(buf), out;
	for (size_t i = 0; i < in.size(); i++) {
		if (in[i] == L'<') {
			size_t j = in.find(L'>', i);
			if (j != wstring::npos) { i = j; continue; }
		}
		out += in[i];
	}
	return out;
}

void paintLink(HWND h, HDC target, const InputState* st) {
	RECT rc;
	GetClientRect(h, &rc);
	int w = rcW(rc), hh = rcH(rc);
	if (w <= 0 || hh <= 0)
		return;
	MemDC mem(target, w, hh);
	HDC dc = mem.dc();
	// để control tự dựng lại bố cục liên kết (phục vụ hit-test) rồi vẽ đè lên
	DefSubclassProc(h, WM_PRINTCLIENT, (WPARAM)dc, PRF_CLIENT | PRF_ERASEBKGND);
	RECT local = { 0, 0, w, hh };
	fillRect(dc, local, bgFor(h));
	LOGFONTW lf;
	HFONT base = controlFont(h);
	if (GetObjectW(base, sizeof(lf), &lf)) {
		lf.lfUnderline = (st && st->hot) ? TRUE : FALSE;
		HFONT f = CreateFontIndirectW(&lf);
		wstring text = linkPlainText(h);
		drawText(dc, text.c_str(), local, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS,
				 (st && st->hot) ? kAccentHover : kAccent, f ? f : base);
		if (f) DeleteObject(f);
	}
	mem.flush(rc.left, rc.top);
}

LRESULT CALLBACK linkSubclassProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ref) {
	InputState* st = (InputState*)ref;
	switch (msg) {
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(h, &ps);
		paintLink(h, dc, st);
		EndPaint(h, &ps);
		return 0;
	}
	case WM_PRINTCLIENT:
		paintLink(h, (HDC)wp, st);
		return 0;
	case WM_MOUSEMOVE:
		trackHot(h, st, false);
		break;
	case WM_MOUSELEAVE:
		leaveHot(h, st, false);
		break;
	case WM_NCDESTROY:
		RemoveWindowSubclass(h, linkSubclassProc, id);
		delete st;
		break;
	}
	return DefSubclassProc(h, msg, wp, lp);
}

void styleLink(HWND h) {
	DWORD_PTR ref = 0;
	if (GetWindowSubclass(h, linkSubclassProc, 1, &ref))
		return;
	SetWindowSubclass(h, linkSubclassProc, 1, (DWORD_PTR)new InputState{ false, false });
	InvalidateRect(h, NULL, TRUE);
}

/* ---------- Thanh tiêu đề: sáng/tối theo chế độ (Windows 10 20H1+ và Windows 11) ---------- */
void applyTitleBar(HWND h, bool dark) {
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
	BOOL value = dark ? TRUE : FALSE;
	// 20 = DWMWA_USE_IMMERSIVE_DARK_MODE; 19 là giá trị cũ của Windows 10 trước bản 20H1. Bản không hỗ trợ sẽ bỏ qua lỗi.
	if (FAILED(fn(h, 20, &value, sizeof(value))))
		fn(h, 19, &value, sizeof(value));
}

/* ---------- Áp dụng cho dialog ---------- */
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
	SetWindowTheme(lv, isDark() ? L"DarkMode_Explorer" : L"Explorer", NULL);
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
		else if (isClass(c, L"Edit"))
			styleEdit(c);
		else if (isClass(c, L"ComboBox"))
			styleCombo(c);
		else if (isClass(c, L"SysLink"))
			styleLink(c);
	}

	// 3) nới nhãn bị thiếu chỗ
	for (size_t i = 0; i < children.size(); i++)
		fitLabel(children, children[i], rcW(pc));
}

/* ---------- Hộp thoại thông báo (thay MessageBox) ---------- */
const int kMsgPadPx      = 24;   // lề trái/phải của vùng nội dung
const int kMsgPadVPx     = 20;   // lề trên/dưới của vùng nội dung
const int kMsgIconPx     = 40;
const int kMsgGapPx      = 16;
const int kMsgMinTextPx  = 200;
const int kMsgMaxTextPx  = 380;
const int kMsgBtnWPx     = 92;
const int kMsgBtnHPx     = 32;
const int kMsgBtnGapPx   = 8;
const int kMsgFootPadPx  = 16;
const int kMsgIconCtl    = 100;

struct MsgBoxParams {
	const wchar_t* text;
	const wchar_t* caption;
	UINT flags;
	HFONT font;
	int icon;              // 0 = không, 1 = thông tin, 2 = cảnh báo, 3 = lỗi, 4 = câu hỏi
	int ids[3];
	const wchar_t* labels[3];
	int count;
	int defIndex;
	int cancelId;          // id trả về khi Esc / nút đóng; 0 = không cho đóng (như MB_YESNO của Windows)
	int contentH;          // chiều cao vùng nội dung; thanh nút nằm bên dưới
};

void msgSetButton(MsgBoxParams& P, int index, int id, const wchar_t* label) {
	P.ids[index] = id;
	P.labels[index] = label;
}

void msgFillButtons(MsgBoxParams& P) {
	switch (P.flags & MB_TYPEMASK) {
	case MB_OKCANCEL:
		P.count = 2; msgSetButton(P, 0, IDOK, L"Đồng ý"); msgSetButton(P, 1, IDCANCEL, L"Hủy");
		P.cancelId = IDCANCEL;
		break;
	case MB_YESNO:
		P.count = 2; msgSetButton(P, 0, IDYES, L"Có"); msgSetButton(P, 1, IDNO, L"Không");
		P.cancelId = 0;
		break;
	case MB_YESNOCANCEL:
		P.count = 3; msgSetButton(P, 0, IDYES, L"Có"); msgSetButton(P, 1, IDNO, L"Không"); msgSetButton(P, 2, IDCANCEL, L"Hủy");
		P.cancelId = IDCANCEL;
		break;
	case MB_RETRYCANCEL:
		P.count = 2; msgSetButton(P, 0, IDRETRY, L"Thử lại"); msgSetButton(P, 1, IDCANCEL, L"Hủy");
		P.cancelId = IDCANCEL;
		break;
	default: // MB_OK
		P.count = 1; msgSetButton(P, 0, IDOK, L"Đồng ý");
		P.cancelId = IDOK;
		break;
	}
	int def = (int)((P.flags & MB_DEFMASK) >> 8);
	P.defIndex = def < P.count ? def : 0;
}

int msgIconKind(UINT flags) {
	switch (flags & MB_ICONMASK) {
	case MB_ICONHAND:        return 3;
	case MB_ICONQUESTION:    return 4;
	case MB_ICONEXCLAMATION: return 2;
	case MB_ICONASTERISK:    return 1;
	}
	return 0;
}

HFONT glyphFont(int heightPx) {
	LOGFONTW lf = { 0 };
	lf.lfHeight = -heightPx;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lstrcpynW(lf.lfFaceName, L"Segoe UI Semibold", LF_FACESIZE);
	return cachedFont(lf);
}

// Biểu tượng tròn + ký hiệu (i, !, ×, ?) vẽ bằng font chữ, không phụ thuộc font biểu tượng của Windows.
void drawMsgIcon(const DRAWITEMSTRUCT* di, const MsgBoxParams* P) {
	HDC dc = di->hDC;
	int w = rcW(di->rcItem), hh = rcH(di->rcItem);
	MemDC mem(dc, w, hh);
	HDC m = mem.dc();
	RECT local = { 0, 0, w, hh };
	fillRect(m, local, kCard);
	bool dark = isDark();
	COLORREF fill = kAccent;
	const wchar_t* glyph = L"i";
	switch (P->icon) {
	case 2: fill = dark ? RGB(0xF6, 0xB4, 0x45) : RGB(0xC7, 0x6A, 0x00); glyph = L"!"; break;
	case 3: fill = kDanger; glyph = L"×"; break;
	case 4: fill = kAccent; glyph = L"?"; break;
	}
	drawEllipse(m, 0, 0, (float)w, (float)hh, fill, CLR_NONE, 0.0f);
	COLORREF ink = dark ? RGB(0x20, 0x20, 0x20) : RGB(0xFF, 0xFF, 0xFF);
	drawText(m, glyph, local, DT_CENTER | DT_VCENTER | DT_SINGLELINE, ink, glyphFont(hh * 6 / 10));
	mem.flush(di->rcItem.left, di->rcItem.top);
}

void msgCenter(HWND hDlg, int W, int H) {
	HWND owner = GetWindow(hDlg, GW_OWNER);
	RECT area;
	HMONITOR mon = MonitorFromWindow(owner ? owner : hDlg, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(mon, &mi))
		area = mi.rcWork;
	else
		SystemParametersInfo(SPI_GETWORKAREA, 0, &area, 0);
	RECT ref = area;
	if (owner && IsWindowVisible(owner) && !IsIconic(owner))
		GetWindowRect(owner, &ref);
	int x = ref.left + (rcW(ref) - W) / 2;
	int y = ref.top + (rcH(ref) - H) / 2;
	if (x + W > area.right) x = area.right - W;
	if (y + H > area.bottom) y = area.bottom - H;
	if (x < area.left) x = area.left;
	if (y < area.top) y = area.top;
	SetWindowPos(hDlg, NULL, x, y, W, H, SWP_NOZORDER | SWP_NOACTIVATE);
}

void msgInit(HWND hDlg, MsgBoxParams* P) {
	SetWindowText(hDlg, P->caption ? P->caption : L"");
	applyTitleBar(hDlg, isDark());

	// đo chữ
	int maxW = px(kMsgMaxTextPx), minW = px(kMsgMinTextPx);
	HDC dc = GetDC(hDlg);
	HFONT old = (HFONT)SelectObject(dc, P->font);
	RECT tr = { 0, 0, maxW, 0 };
	DrawTextW(dc, P->text ? P->text : L"", -1, &tr, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
	SelectObject(dc, old);
	ReleaseDC(hDlg, dc);
	int textW = tr.right < minW ? minW : tr.right;
	int textH = tr.bottom;

	int iconD = P->icon ? px(kMsgIconPx) : 0;
	int gap = P->icon ? px(kMsgGapPx) : 0;
	int pad = px(kMsgPadPx), padV = px(kMsgPadVPx);
	int innerH = textH > iconD ? textH : iconD;
	P->contentH = padV + innerH + padV;
	int contentW = pad + iconD + gap + textW + pad;

	int btnW = px(kMsgBtnWPx), btnH = px(kMsgBtnHPx), btnGap = px(kMsgBtnGapPx), footPad = px(kMsgFootPadPx);
	int rowW = P->count * btnW + (P->count - 1) * btnGap;
	int clientW = contentW > rowW + 2 * footPad ? contentW : rowW + 2 * footPad;
	int clientH = P->contentH + btnH + 2 * footPad;
	textW += clientW - contentW; // chữ chiếm hết bề ngang còn lại

	HINSTANCE inst = GetModuleHandle(NULL);
	int x = pad;
	if (P->icon) {
		HWND ic = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, x, padV + (innerH - iconD) / 2, iconD, iconD,
								  hDlg, (HMENU)(INT_PTR)kMsgIconCtl, inst, NULL);
		SetPropW(ic, kPropCard, (HANDLE)1);
		x += iconD + gap;
	}
	HWND label = CreateWindowExW(0, L"STATIC", P->text ? P->text : L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
								 x, padV + (innerH - textH) / 2, textW, textH + px(2), hDlg, (HMENU)(INT_PTR)-1, inst, NULL);
	SendMessage(label, WM_SETFONT, (WPARAM)P->font, TRUE);
	SetPropW(label, kPropCard, (HANDLE)1);

	int bx = clientW - footPad - rowW;
	int by = P->contentH + footPad;
	HWND defBtn = NULL;
	for (int i = 0; i < P->count; i++) {
		bool isDef = i == P->defIndex;
		HWND b = CreateWindowExW(0, L"BUTTON", P->labels[i],
								 WS_CHILD | WS_VISIBLE | WS_TABSTOP | (isDef ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
								 bx, by, btnW, btnH, hDlg, (HMENU)(INT_PTR)P->ids[i], inst, NULL);
		SendMessage(b, WM_SETFONT, (WPARAM)P->font, TRUE);
		setButtonKind(b, isDef ? ButtonKind::Primary : ButtonKind::Secondary);
		if (isDef) defBtn = b;
		bx += btnW + btnGap;
	}
	if (defBtn)
		SendMessage(hDlg, DM_SETDEFID, P->ids[P->defIndex], 0);

	RECT wr = { 0, 0, clientW, clientH };
	AdjustWindowRectEx(&wr, GetWindowLong(hDlg, GWL_STYLE), FALSE, GetWindowLong(hDlg, GWL_EXSTYLE));
	msgCenter(hDlg, rcW(wr), rcH(wr));

	switch (P->icon) {
	case 1: MessageBeep(MB_ICONASTERISK); break;
	case 2: MessageBeep(MB_ICONEXCLAMATION); break;
	case 3: MessageBeep(MB_ICONHAND); break;
	case 4: MessageBeep(MB_ICONQUESTION); break;
	}
	if (defBtn)
		SetFocus(defBtn);
}

INT_PTR CALLBACK msgBoxProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {
	MsgBoxParams* P = (MsgBoxParams*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lp);
		msgInit(hDlg, (MsgBoxParams*)lp);
		return FALSE; // đã tự đặt tiêu điểm
	case WM_ERASEBKGND: {
		if (!P) break;
		HDC dc = (HDC)wp;
		RECT rc;
		GetClientRect(hDlg, &rc);
		RECT content = { 0, 0, rc.right, P->contentH };
		RECT line = { 0, P->contentH, rc.right, P->contentH + 1 };
		fillRect(dc, rc, kBgMain);
		fillRect(dc, content, kCard);
		fillRect(dc, line, kBorder);
		return TRUE;
	}
	case WM_DRAWITEM:
		if (P && wp == kMsgIconCtl) {
			drawMsgIcon((const DRAWITEMSTRUCT*)lp, P);
			return TRUE;
		}
		break;
	case WM_COMMAND: {
		if (!P) break;
		int id = LOWORD(wp);
		if (id == IDCANCEL) { // Esc, hoặc nút Hủy
			if (P->cancelId)
				EndDialog(hDlg, P->cancelId);
			return TRUE;
		}
		for (int i = 0; i < P->count; i++) {
			if (id == P->ids[i]) {
				EndDialog(hDlg, id);
				return TRUE;
			}
		}
		break;
	}
	case WM_CLOSE:
		if (P && P->cancelId)
			EndDialog(hDlg, P->cancelId);
		return TRUE;
	}
	INT_PTR result = 0;
	if (handleMessage(hDlg, msg, wp, lp, result))
		return result;
	return FALSE;
}

// Khi đổi chế độ sáng/tối: các control có theme riêng (ListView, danh sách thả xuống) phải đặt lại.
BOOL CALLBACK restyleChildProc(HWND c, LPARAM) {
	if (isClass(c, L"SysListView32"))
		styleListView(c);
	else if (isClass(c, L"ComboBox"))
		themeComboList(c);
	return TRUE;
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
	ensureTheme();
	applyTitleBar(hDlg, isDark());
	prepareChildren(hDlg, false);
}

/* ---------- Chế độ sáng / tối ---------- */
void setMode(int mode) {
	if (mode < 0 || mode > 2)
		mode = 0;
	g_mode = mode;
	g_darkCache = -1;
}

int getMode() {
	return g_mode;
}

bool isDark() {
	if (g_darkCache < 0)
		g_darkCache = computeDark() ? 1 : 0;
	return g_darkCache == 1;
}

void refresh(HWND hTop) {
	if (!isEnabled())
		return;
	ensureInit();
	g_darkCache = -1;
	ensureTheme();
	if (!hTop)
		return;
	applyTitleBar(hTop, isDark());
	EnumChildWindows(hTop, restyleChildProc, 0);
	RedrawWindow(hTop, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

int messageBox(HWND parent, const wchar_t* text, const wchar_t* caption, UINT flags) {
	UINT type = flags & MB_TYPEMASK;
	bool supported = type == MB_OK || type == MB_OKCANCEL || type == MB_YESNO || type == MB_YESNOCANCEL || type == MB_RETRYCANCEL;
	if (!isEnabled() || !supported || (flags & MB_HELP))
		return MessageBoxW(parent, text, caption, flags); // theme tắt hoặc kiểu nút không hỗ trợ: dùng của Windows
	ensureInit();
	ensureTheme();

	MsgBoxParams P;
	ZeroMemory(&P, sizeof(P));
	P.text = text;
	P.caption = caption;
	P.flags = flags;
	P.icon = msgIconKind(flags);
	msgFillButtons(P);
	NONCLIENTMETRICSW ncm;
	ZeroMemory(&ncm, sizeof(ncm));
	ncm.cbSize = sizeof(ncm);
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
		P.font = CreateFontIndirectW(&ncm.lfMessageFont);
	if (!P.font)
		P.font = (HFONT)GetStockObject(DEFAULT_GUI_FONT); // không xóa: font hệ thống

	struct {
		DLGTEMPLATE dt;
		WORD menu;
		WORD cls;
		WORD title;
	} tmpl;
	ZeroMemory(&tmpl, sizeof(tmpl));
	tmpl.dt.style = WS_POPUP | WS_CAPTION | DS_MODALFRAME | (P.cancelId ? WS_SYSMENU : 0) | ((parent == NULL || (flags & MB_SETFOREGROUND)) ? DS_SETFOREGROUND : 0);
	tmpl.dt.dwExtendedStyle = (flags & MB_TOPMOST) ? WS_EX_TOPMOST : 0;
	tmpl.dt.cx = 100;
	tmpl.dt.cy = 50;

	INT_PTR r = DialogBoxIndirectParamW(GetModuleHandle(NULL), (LPCDLGTEMPLATE)&tmpl, parent, msgBoxProc, (LPARAM)&P);
	if (P.font != (HFONT)GetStockObject(DEFAULT_GUI_FONT))
		DeleteObject(P.font);
	if (r == -1 || r == 0)
		return MessageBoxW(parent, text, caption, flags);
	return (int)r;
}


void markPage(HWND hPage) {
	SetPropW(hPage, kPropPage, (HANDLE)1);
}

void applyPage(HWND hPage) {
	if (!isEnabled())
		return;
	ensureInit();
	ensureTheme();
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

bool handleMessage(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp, INT_PTR& result) {
	switch (msg) {
	case WM_THEMECHANGED:
	case WM_SYSCOLORCHANGE:
		g_enabledCache = -1;
		g_darkCache = -1;
		return false;
	case WM_SETTINGCHANGE:
		g_enabledCache = -1;
		g_darkCache = -1;
		// Windows đổi giao diện sáng/tối khi dialog đang mở (chỉ khi đang "theo Windows"): đổi theo ngay
		if (lp != 0 && g_mode == 0 && g_appliedDark >= 0 && lstrcmpW((LPCWSTR)lp, L"ImmersiveColorSet") == 0 && isEnabled()) {
			if ((computeDark() ? 1 : 0) != g_appliedDark)
				refresh(hDlg);
		}
		return false;
	}
	if (!isEnabled())
		return false;
	ensureInit();
	ensureTheme();

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
			color = kTextPrimary;
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

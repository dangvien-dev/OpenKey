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
#pragma once
#include "stdafx.h"

/*
 * Theme - owner-draw skin cho các dialog Win32 của OpenKey.
 *
 * Phong cách "êm mắt, tinh gọn" cho phần mềm dùng nhiều giờ liền (cùng tinh thần
 * trang Settings của Windows 11 / Office): nền xám nhạt trung tính, card trắng
 * viền mảnh, chữ gần đen, MỘT màu nhấn xanh dịu cho hành động chính, bo góc nhỏ,
 * thanh tiêu đề chuẩn của Windows, icon Segoe MDL2 Assets.
 *
 * Nguyên tắc:
 *  - Không đổi hành vi: mọi control vẫn là control gốc của Windows
 *    (BM_GETCHECK, BN_CLICKED, TCN_SELCHANGE... giữ nguyên). Theme chỉ
 *    vẽ lại bằng NM_CUSTOMDRAW / WM_CTLCOLOR* / window subclass.
 *  - Khi visual styles tắt hoặc đang bật High Contrast thì isEnabled() = false
 *    và toàn bộ giao diện rơi về kiểu Windows mặc định (tốt cho khả năng tiếp cận).
 */
namespace Theme {

/* ---------- Design tokens (COLORREF) ---------- */
// Nền / bề mặt
const COLORREF kBgMain        = RGB(0xF3, 0xF3, 0xF3);
const COLORREF kCard          = RGB(0xFF, 0xFF, 0xFF);
const COLORREF kBorder        = RGB(0xE5, 0xE5, 0xE5);
const COLORREF kBorderInput   = RGB(0xD1, 0xD1, 0xD1);
const COLORREF kControlBorder = RGB(0x70, 0x70, 0x70); // viền checkbox/radio (4.9:1 trên nền trắng)
// Chữ
const COLORREF kTextPrimary   = RGB(0x1A, 0x1A, 0x1A);
const COLORREF kTextSecondary = RGB(0x61, 0x61, 0x61); // chỉ dùng trên nền trắng (6.2:1)
const COLORREF kTextMuted     = RGB(0x5C, 0x5C, 0x5C); // thay cho kTextSecondary khi nền là kBgMain (5.6:1)
const COLORREF kTextDisabled  = RGB(0xA6, 0xA6, 0xA6);
// Hành động (một màu nhấn duy nhất)
const COLORREF kAccent        = RGB(0x0F, 0x6C, 0xBD); // nút chính, checkbox/radio đã chọn, gạch chân tab (5.4:1 trên trắng)
const COLORREF kAccentHover   = RGB(0x11, 0x5E, 0xA3);
const COLORREF kAccentPressed = RGB(0x0F, 0x54, 0x8C);
const COLORREF kAccentSoft    = RGB(0xEB, 0xF3, 0xFC);
// Nguy hiểm (chỉ tô khi rê chuột; mặc định nút vẫn trung tính)
const COLORREF kDanger        = RGB(0xC4, 0x2B, 0x1C);
const COLORREF kDangerText    = RGB(0xC4, 0x2B, 0x1C); // 5.7:1 trên nền trắng
const COLORREF kDangerSoft    = RGB(0xFD, 0xE7, 0xE9);
const COLORREF kDangerSoftPressed = RGB(0xF9, 0xD2, 0xD5);

enum class ButtonKind {
	Auto = 0,   // Secondary (viền accent nếu là nút mặc định của dialog)
	Primary,    // nền xanh, chữ trắng
	Secondary,  // nền trắng, viền xám
	Danger      // viền/chữ đỏ, nền đỏ nhạt khi hover
};

enum class LabelRole {
	Body = 0,
	Title,      // màu kTextPrimary
	Muted       // chữ phụ
};

/* ---------- Vòng đời ---------- */
// true khi được phép vẽ giao diện tùy biến (visual styles bật, không High Contrast).
bool isEnabled();

// Gọi 1 lần sau WM_INITDIALOG của dialog top-level (BaseDialog tự gọi).
void apply(HWND hDlg);

// Dành cho dialog con làm trang của tab: nền trắng (card).
// markPage gọi ngay trong WM_INITDIALOG của trang, applyPage gọi SAU KHI đã đặt kích thước trang.
void markPage(HWND hPage);
void applyPage(HWND hPage);

// Chuyển tiếp message của dialog (top-level hoặc trang tab) cho Theme.
// Trả về true nếu Theme đã xử lý; khi đó `result` là giá trị dialog proc phải trả về.
bool handleMessage(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam, INT_PTR& result);

/* ---------- Tùy biến từng control ---------- */
void setButtonKind(HWND hButton, ButtonKind kind, const wchar_t* glyph = nullptr);
void setLabelRole(HWND hLabel, LabelRole role);

/* ---------- Tab control ---------- */
// Biến SysTabControl32 thành tab gạch chân + card. Trả về chiều cao header (px),
// hoặc 0 nếu theme đang tắt (khi đó caller dùng cách tính cũ).
int styleTabControl(HWND hTab);
// Vùng (tọa độ client của hParent) để đặt các trang tab.
RECT tabPageRect(HWND hTab, HWND hParent);

/* ---------- Vẽ owner-draw ---------- */
// Quy đổi px @96dpi sang px theo DPI hệ thống (app khai báo dpiAware).
int dpi(int px96);

}

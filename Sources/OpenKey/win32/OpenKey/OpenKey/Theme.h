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
 * Ngôn ngữ thiết kế lấy từ TechMaster Pro (Styles/Colors.xaml, HANDOFF.md mục 3):
 * nền #F1F5F9, card trắng, viền #E2E8F0, chữ #1E293B, nút chính #2563EB,
 * thanh tiêu đề/sidebar #0F172A, icon Segoe MDL2 Assets.
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
const COLORREF kSidebar       = RGB(0x0F, 0x17, 0x2A); // header band + thanh tiêu đề
const COLORREF kBgMain        = RGB(0xF1, 0xF5, 0xF9);
const COLORREF kCard          = RGB(0xFF, 0xFF, 0xFF);
const COLORREF kBorder        = RGB(0xE2, 0xE8, 0xF0);
const COLORREF kBorderInput   = RGB(0xCB, 0xD5, 0xE1);
const COLORREF kControlBorder = RGB(0x64, 0x74, 0x8B); // viền checkbox/radio (>= 3:1 trên nền trắng)
// Chữ
const COLORREF kTextPrimary   = RGB(0x1E, 0x29, 0x3B);
const COLORREF kTextSecondary = RGB(0x64, 0x74, 0x8B); // chỉ dùng trên nền trắng (4.76:1)
const COLORREF kTextMuted     = RGB(0x47, 0x55, 0x69); // thay cho kTextSecondary khi nền là kBgMain
const COLORREF kTextDisabled  = RGB(0x94, 0xA3, 0xB8);
const COLORREF kTextOnDark    = RGB(0xF8, 0xFA, 0xFC);
const COLORREF kTextOnDarkMuted = RGB(0x94, 0xA3, 0xB8);
// Thương hiệu / hành động
const COLORREF kPrimary       = RGB(0x1E, 0x3A, 0x8A); // tiêu đề, thương hiệu
const COLORREF kAccent        = RGB(0x25, 0x63, 0xEB); // PrimaryButton
const COLORREF kAccentHover   = RGB(0x1D, 0x4E, 0xD8);
const COLORREF kAccentPressed = RGB(0x1E, 0x40, 0xAF);
const COLORREF kAccentSoft    = RGB(0xEF, 0xF6, 0xFF);
// Trạng thái
const COLORREF kSuccess       = RGB(0x22, 0xC5, 0x5E);
const COLORREF kWarning       = RGB(0xFA, 0xCC, 0x15);
const COLORREF kDanger        = RGB(0xEF, 0x44, 0x44);
const COLORREF kDangerText    = RGB(0xB9, 0x1C, 0x1C); // chữ đỏ đạt AA trên nền trắng
const COLORREF kDangerSoft    = RGB(0xFE, 0xF2, 0xF2);
const COLORREF kDangerSoftPressed = RGB(0xFE, 0xE2, 0xE2);

enum class ButtonKind {
	Auto = 0,   // Secondary (viền accent nếu là nút mặc định của dialog)
	Primary,    // nền xanh, chữ trắng
	Secondary,  // nền trắng, viền xám
	Danger      // viền/chữ đỏ, nền đỏ nhạt khi hover
};

enum class LabelRole {
	Body = 0,
	Title,      // màu kPrimary
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

// Header band phía trên cùng cửa sổ chính: icon + tên app + mô tả + "pill" trạng thái.
void paintHeader(HDC hdc, const RECT& rc, HICON icon, const wchar_t* title,
				 const wchar_t* subtitle, const wchar_t* status, bool statusOn, HFONT baseFont);

}

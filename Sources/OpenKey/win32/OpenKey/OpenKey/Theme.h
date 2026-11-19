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

/* ---------- Design tokens (COLORREF) ----------
 * Là biến (không phải hằng) vì đổi theo chế độ sáng/tối lúc chạy; giá trị đặt trong Theme.cpp (palette sáng và tối).
 */
// Nền / bề mặt
extern COLORREF kBgMain;
extern COLORREF kCard;
extern COLORREF kBorder;
extern COLORREF kBorderInput;
extern COLORREF kControlBorder; // viền checkbox/radio (4.9:1 trên nền trắng)
// Chữ
extern COLORREF kTextPrimary;
extern COLORREF kTextSecondary; // chỉ dùng trên nền trắng (6.2:1)
extern COLORREF kTextMuted; // thay cho kTextSecondary khi nền là kBgMain (5.6:1)
extern COLORREF kTextDisabled;
// Hành động (một màu nhấn duy nhất)
extern COLORREF kAccent; // nút chính, checkbox/radio đã chọn, gạch chân tab (5.4:1 trên trắng)
extern COLORREF kAccentHover;
extern COLORREF kAccentPressed;
extern COLORREF kAccentSoft;
extern COLORREF kOnAccent;     // chữ/dấu tick đặt trên nền kAccent
extern COLORREF kHover;        // nền nút phụ khi rê chuột
// Nguy hiểm (chỉ tô khi rê chuột; mặc định nút vẫn trung tính)
extern COLORREF kDanger;
extern COLORREF kDangerText; // 5.7:1 trên nền trắng
extern COLORREF kDangerSoft;
extern COLORREF kDangerSoftPressed;

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

/* ---------- Chế độ sáng / tối ---------- */
// 0 = theo Windows (mặc định), 1 = luôn sáng, 2 = luôn tối. Lưu ở registry do ứng dụng quản lý; Theme chỉ nhận giá trị.
void setMode(int mode);
int getMode();
// true khi đang dùng bảng màu tối (đã tính cả chế độ ứng dụng của Windows khi mode = 0).
bool isDark();
// Tính lại bảng màu và vẽ lại toàn bộ dialog `hTop` cùng mọi control con (dùng khi người dùng đổi chế độ,
// hoặc Windows đổi sáng/tối lúc dialog đang mở).
void refresh(HWND hTop);

/* ---------- Hộp thoại thông báo ---------- */
// Thay cho MessageBox: cùng phong cách với giao diện (biểu tượng, nút bo góc, nền sáng/tối).
// Hỗ trợ MB_OK, MB_OKCANCEL, MB_YESNO, MB_YESNOCANCEL, MB_RETRYCANCEL và MB_ICONINFORMATION/WARNING/ERROR/QUESTION.
// Trả về IDOK, IDCANCEL, IDYES, IDNO hoặc IDRETRY. Khi theme tắt (High Contrast...) rơi về MessageBox của Windows.
int messageBox(HWND parent, const wchar_t* text, const wchar_t* caption, UINT flags);

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

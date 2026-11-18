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
#include "MainControlDialog.h"
#include "AboutDialog.h"
#include "ConvertToolDialog.h"
#include "MacroDialog.h"

//luồng nền báo cho cửa sổ ẩn (luồng chính) biết có bản mới; lParam là std::string* tên phiên bản (nơi nhận giải phóng)
#define WM_APP_UPDATE_AVAILABLE (WM_USER + 2020)

class BaseDialog;
class AppDelegate {
private:
	HINSTANCE hInstance;
	BaseDialog* mainDialog = NULL, *aboutDialog = NULL, *macroDialog = NULL, *convertDialog = NULL;
private:
	bool isDialogMsg(MSG &msg) const;
	void checkUpdate();
public:
	AppDelegate();
	static AppDelegate* getInstance();
	int run(HINSTANCE hInstance);
	void createMainDialog();
	void closeDialog(BaseDialog* dialog);
public: //event
	void onInputMethodChangedFromHotKey();
	void onDefaultConfig();

	void onToggleVietnamese();
	void onToggleCheckSpelling();
	void onToggleUseSmartSwitchKey();
	void onToggleUseMacro();

	void onMacroTable();
	void onConvertTool();
	void onQuickConvert();

	void onInputType(const int& type);
	void onTableCode(const int& code);

	void onControlPanel();
	void onOpenKeyAbout();
	void onOpenKeyExit();
	void onUpdateAvailable(std::string* newVersion);
};
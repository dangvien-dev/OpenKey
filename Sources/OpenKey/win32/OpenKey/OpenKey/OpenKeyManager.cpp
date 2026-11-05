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
#include "OpenKeyManager.h"
#include <shlobj.h>

static vector<LPCTSTR> _inputType = {
	_T("Telex"),
	_T("VNI"),
	_T("Simple Telex"),
};

static vector<LPCTSTR> _tableCode = {
	_T("Unicode"),
	_T("TCVN3 (ABC)"),
	_T("VNI Windows"),
	_T("Unicode Tổ hợp"),
	_T("Vietnamese Locale CP 1258")
};

/*-----------------------------------------------------------------------*/

extern void OpenKeyInit();
extern void OpenKeyFree();

unsigned short  OpenKeyManager::_lastKeyCode = 0;

vector<LPCTSTR>& OpenKeyManager::getInputType() {
	return _inputType;
}

vector<LPCTSTR>& OpenKeyManager::getTableCode() {
	return _tableCode;
}

void OpenKeyManager::initEngine() {
	OpenKeyInit();
}

void OpenKeyManager::freeEngine() {
	OpenKeyFree();
}

bool OpenKeyManager::checkUpdate(string& newVersion) {
	wstring dataW = OpenKeyHelper::getContentOfUrl(L"https://raw.githubusercontent.com/tuyenvm/OpenKey/master/version.json");
	string data = wideStringToUtf8(dataW);

	//simple parse
	constexpr char versionNameStr[] = "\"versionName\":";
	constexpr char versionCodeStr[] = "\"versionCode\":";
	constexpr char numbers[] = "0123456789";
	size_t posBegin = string::npos;
	size_t posEnd = string::npos;

	//every find() is checked before it is used: a failed download (empty data) or an unexpected
	//document must not turn npos into an offset
	posBegin = data.find("latestWinVersion");
	if (posBegin == string::npos) {
		return false;
	}
	posBegin = data.find(versionNameStr, posBegin);
	if (posBegin == string::npos) {
		return false;
	}
	posBegin += (sizeof(versionNameStr) - 1);
	posBegin = data.find('\"', posBegin);
	if (posBegin == string::npos) {
		return false;
	}
	posBegin = data.find_first_of(numbers, posBegin);
	if (posBegin == string::npos) {
		return false;
	}

	posEnd = data.find('\"', posBegin);
	if (posEnd == string::npos) {
		return false;
	}

	newVersion = data.substr(posBegin, posEnd - posBegin);

	posBegin = data.find(versionCodeStr, posEnd);
	if (posBegin == string::npos) {
		return false;
	}
	posBegin += (sizeof(versionCodeStr) - 1);

	posEnd = data.find("}", posBegin);
	if (posEnd == string::npos) {
		return false;
	}

	//version code layout (see OpenKeyHelper::getVersionNumber): major | minor << 8 | build << 16.
	//Reorder it so that major is the most significant byte, then minor, then build.
	//Each byte has to be masked, otherwise minor leaks into the top byte and a major bump
	//(e.g. 2.1.0 -> 3.0.0) is not detected as an update.
	auto shiftVersion = [](DWORD version) {
		return ((version & 0xFF) << 16) | (version & 0x00FF00) | ((version >> 16) & 0xFF);
		};

	string newVersionCodeStr = data.substr(posBegin, posEnd - posBegin);
	DWORD newVersionCode = (DWORD)atoi(newVersionCodeStr.data());
	newVersionCode = shiftVersion(newVersionCode);

	DWORD currentVersionCode = OpenKeyHelper::getVersionNumber();
	currentVersionCode = shiftVersion(currentVersionCode);

	return newVersionCode > currentVersionCode;
}

void OpenKeyManager::createDesktopShortcut() {
	CoInitialize(NULL);
	IShellLink* pShellLink = NULL;
	HRESULT hres;
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_ALL,
							IID_IShellLink, (void**)&pShellLink);
	if (SUCCEEDED(hres)) {
		wstring path = OpenKeyHelper::getFullPath();
		pShellLink->SetPath(path.c_str());
		pShellLink->SetDescription(_T("OpenKey - Bộ gõ Tiếng Việt"));
		pShellLink->SetIconLocation(path.c_str(), 0);

		IPersistFile* pPersistFile;
		hres = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);

		if (SUCCEEDED(hres)) {
			wchar_t desktopPath[MAX_PATH + 1];
			wchar_t savePath[MAX_PATH + 10];
			SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath);
			wsprintf(savePath, _T("%s\\OpenKey.lnk"), desktopPath);
			hres = pPersistFile->Save(savePath, TRUE);
			pPersistFile->Release();
			pShellLink->Release();
		}
	}
}

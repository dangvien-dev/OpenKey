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
#include "OpenKeyHelper.h"
#include <stdarg.h>
#include <Urlmon.h>
#include <fstream>
#include <sstream>

#pragma comment(lib, "version.lib")
#pragma comment(lib, "Urlmon.lib")

static BYTE* _regData = 0;

static LPCTSTR sk = TEXT("SOFTWARE\\TuyenMai\\OpenKey");
static HKEY hKey;
static LPCTSTR _runOnStartupKeyPath = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
static TCHAR _executePath[MAX_PATH];
static bool _hasGetPath = false;

static DWORD _cacheProcessId = 0, _tempProcessId = 0;
static HWND _tempWnd;
static TCHAR _exePath[1024] = { 0 };
static LPCTSTR _exeName = _exePath;
static HANDLE _proc;
static string _exeNameUtf8 = "TheOpenKeyProject";
static string _unknownProgram = "UnknownProgram";

int CF_RTF = RegisterClipboardFormat(_T("Rich Text Format"));
int CF_HTML = RegisterClipboardFormat(_T("HTML Format"));
int CF_OPENKEY = RegisterClipboardFormat(_T("OpenKey Format"));

void OpenKeyHelper::openKey() {
	LONG nError = RegOpenKeyEx(HKEY_CURRENT_USER, sk, NULL, KEY_ALL_ACCESS, &hKey);
	if (nError == ERROR_FILE_NOT_FOUND) 	{
		nError = RegCreateKeyEx(HKEY_CURRENT_USER, sk, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL, &hKey, NULL);
	}
	if (nError) {
		LOG(L"result %d\n", nError);
	}
}

void OpenKeyHelper::setRegInt(LPCTSTR key, const int & val) {
	openKey();
	RegSetValueEx(hKey, key, 0, REG_DWORD, (LPBYTE)&val, sizeof(val));
	RegCloseKey(hKey);
}

int OpenKeyHelper::getRegInt(LPCTSTR key, const int & defaultValue) {
	openKey();
	int val = defaultValue;
	DWORD size = sizeof(val);
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, key, 0, 0, (LPBYTE)&val, &size)) {
		val = defaultValue;
	}
	RegCloseKey(hKey);
	return val;
}

void OpenKeyHelper::setRegBinary(LPCTSTR key, const BYTE * pData, const int & size) {
	openKey();
	RegSetValueEx(hKey, key, 0, REG_BINARY, pData, size);
	RegCloseKey(hKey);
}

BYTE * OpenKeyHelper::getRegBinary(LPCTSTR key, DWORD& outSize) {
	openKey();
	if (_regData) {
		delete[] _regData;
		_regData = NULL;
	}
	DWORD size = 0;
	RegQueryValueEx(hKey, key, 0, 0, 0, &size);
	_regData = new BYTE[size];
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, key, 0, 0, _regData, &size)) {
		delete[] _regData;
		_regData = NULL;
	}
	outSize = size;
	RegCloseKey(hKey);
	return _regData;
}

//Start a hidden process without waiting for it.
//Replaces WinExec, which only takes an ANSI command line: a UTF-8 path containing
//non-ASCII characters (a user folder with Vietnamese diacritics) was turned into garbage.
//(keep this file ASCII-only: it has no BOM, MSVC would read it with the ANSI code page)
static void runHidden(const wstring& commandLine) {
	wstring cmd = commandLine; //CreateProcessW may modify the buffer
	STARTUPINFOW si = { sizeof(si) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = {};
	if (CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
}

void OpenKeyHelper::registerRunOnStartup(const int& val) {
	HKEY hRunKey = NULL;
	if (val) {
		//quote the path: it may contain spaces
		wstring quotedPath = L"\"" + getFullPath() + L"\"";
		if (vRunAsAdmin) {
			//schtasks needs the inner quotes escaped: /tr "\"C:\path with space\OpenKey.exe\""
			runHidden(L"schtasks /create /sc onlogon /tn OpenKey /rl highest /tr \"\\\"" + getFullPath() + L"\\\"\" /f");
		} else {
			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, _runOnStartupKeyPath, 0, KEY_SET_VALUE, &hRunKey)) {
				RegSetValueEx(hRunKey, _T("OpenKey"), 0, REG_SZ, (const BYTE*)quotedPath.c_str(), ((DWORD)quotedPath.size() + 1) * sizeof(TCHAR));
				RegCloseKey(hRunKey);
			}
		}
	} else {
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, _runOnStartupKeyPath, 0, KEY_SET_VALUE, &hRunKey)) {
			RegDeleteValue(hRunKey, _T("OpenKey"));
			RegCloseKey(hRunKey);
		}
		runHidden(L"schtasks /delete /tn OpenKey /f");
	}
}

LPTSTR OpenKeyHelper::getExecutePath() {
	if (!_hasGetPath) {
		HMODULE hModule = GetModuleHandleW(NULL);
		GetModuleFileNameW(hModule, _executePath, MAX_PATH);
		_hasGetPath = true;
	}
	return _executePath;
}

string& OpenKeyHelper::getFrontMostAppExecuteName() {
	_tempWnd = GetForegroundWindow();
	_tempProcessId = 0;
	GetWindowThreadProcessId(_tempWnd, &_tempProcessId);
	if (_tempProcessId == 0) { //no foreground window right now (e.g. while switching): keep the last known app
		return _exeNameUtf8;
	}
	if (_tempProcessId == _cacheProcessId) {
		return _exeNameUtf8;
	}
	_cacheProcessId = _tempProcessId;
	//Clear first: if the query below fails, _exePath would otherwise keep the PREVIOUS app's path
	//and the smart-switch settings of that app would be applied to this one.
	_exePath[0] = 0;
	//PROCESS_QUERY_LIMITED_INFORMATION is enough for GetProcessImageFileName and, unlike
	//PROCESS_QUERY_INFORMATION, also works for elevated processes when OpenKey is not run as admin.
	_proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, _tempProcessId);
	if (_proc != NULL) {
		GetProcessImageFileName((HMODULE)_proc, _exePath, 1024);
		CloseHandle(_proc);
	}

	if (wcscmp(_exePath, _T("")) == 0) {
		_cacheProcessId = 0; //don't cache a failed lookup, otherwise the cached branch above returns the previous app's name
		return _unknownProgram;
	}
	LPCTSTR lastSlash = _tcsrchr(_exePath, '\\');
	_exeName = lastSlash ? lastSlash + 1 : _exePath;
	if (wcscmp(_exeName, _T("OpenKey64.exe")) == 0 ||
		wcscmp(_exeName, _T("OpenKey32.exe")) == 0 || 
		wcscmp(_exeName, _T("explorer.exe")) == 0) {
		return _exeNameUtf8;
	}
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, _exeName, (int)lstrlen(_exeName), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, _exeName, (int)lstrlen(_exeName), &strTo[0], size_needed, NULL, NULL);
	_exeNameUtf8 = strTo;
	//LOG(L"%s\n", utf8ToWideString(_exeNameUtf8).c_str());
	return _exeNameUtf8;
}

string & OpenKeyHelper::getLastAppExecuteName() {
	if (!vUseSmartSwitchKey)
		return getFrontMostAppExecuteName();
	return _exeNameUtf8;
}

wstring OpenKeyHelper::getFullPath() {
	HMODULE hModule = GetModuleHandle(NULL);
	TCHAR path[MAX_PATH];
	GetModuleFileName(hModule, path, MAX_PATH);
	wstring rs(path);
	return rs;
}

wstring OpenKeyHelper::getClipboardText(const int& type) {
	// Try opening the clipboard
	if (!OpenClipboard(nullptr)) {
		return _T("");
	}

	// Get handle of clipboard object for ANSI text
	HANDLE hData = GetClipboardData(type);
	if (hData == nullptr) {
		return _T("");
	}

	// Lock the handle to get the actual text pointer
	wchar_t * pszText = static_cast<wchar_t*>(GlobalLock(hData));
	if (pszText == nullptr) {
		return _T("");
	}

	// Save text in a string class instance
	wstring text(pszText);
	
	// Release the lock
	GlobalUnlock(hData);

	// Release the clipboard
	CloseClipboard();
	
	return text;
}

bool OpenKeyHelper::setClipboardText(LPCTSTR data, const int & len, const int& type) {
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(WCHAR));
	if (hMem == NULL) {
		return false;
	}
	void* pMem = GlobalLock(hMem);
	if (pMem == NULL) {
		GlobalFree(hMem);
		return false;
	}
	memcpy(pMem, data, len * sizeof(WCHAR));
	GlobalUnlock(hMem);

	//another process (clipboard manager, remote desktop,...) may hold the clipboard for a few ms
	bool opened = false;
	for (int attempt = 0; attempt < 5 && !opened; attempt++) {
		opened = OpenClipboard(NULL) != FALSE;
		if (!opened) {
			Sleep(2);
		}
	}
	if (!opened) {
		GlobalFree(hMem);
		return false;
	}
	EmptyClipboard();
	bool ok = SetClipboardData(type, hMem) != NULL;
	CloseClipboard();
	if (!ok) {
		GlobalFree(hMem); //the system only takes ownership when SetClipboardData succeeds
	}
	return ok;
}

bool OpenKeyHelper::quickConvert() {
	//read data from clipboard
	//support Unicode raw string, Rich Text Format and HTML

	if (!OpenClipboard(nullptr)) {
		return false;
	}

	string dataHTML;
	wstring dataUnicode;

	//HTML; read the data while the memory is locked
	HANDLE hData = GetClipboardData(CF_HTML);
	if (hData) {
		const char* pHTML = static_cast<const char*>(GlobalLock(hData));
		if (pHTML) {
			dataHTML = convertUtil(string(pHTML));
			GlobalUnlock(hData);
		}
	}

	//UNICODE
	hData = GetClipboardData(CF_UNICODETEXT);
	if (hData) {
		const wchar_t* pUnicode = static_cast<const wchar_t*>(GlobalLock(hData));
		if (pUnicode) {
			dataUnicode = utf8ToWideString(convertUtil(wideStringToUtf8(wstring(pUnicode))));
			GlobalUnlock(hData);
		}
	}

	//Nothing we can convert (empty clipboard, image, file list,...): leave the clipboard as it is,
	//EmptyClipboard() below would destroy the user's data.
	if (dataHTML.empty() && dataUnicode.empty()) {
		CloseClipboard();
		return false;
	}

	//the clipboard is already open (OpenClipboard above)
	EmptyClipboard();

	//Only put back the formats that were really on the clipboard: an empty CF_HTML entry makes
	//rich-text targets (Word, browsers) paste nothing instead of falling back to the plain text.
	auto putData = [](UINT format, const void* src, size_t bytes) {
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
		if (hMem == NULL) {
			return;
		}
		void* pMem = GlobalLock(hMem);
		if (pMem == NULL) {
			GlobalFree(hMem);
			return;
		}
		memcpy(pMem, src, bytes);
		GlobalUnlock(hMem);
		if (SetClipboardData(format, hMem) == NULL) {
			GlobalFree(hMem);
		}
	};
	if (!dataHTML.empty()) {
		putData(CF_HTML, dataHTML.c_str(), (dataHTML.size() + 1) * sizeof(char));
	}
	if (!dataUnicode.empty()) {
		putData(CF_UNICODETEXT, dataUnicode.c_str(), (dataUnicode.size() + 1) * sizeof(wchar_t));
	}

	CloseClipboard();
	return true;
}

DWORD OpenKeyHelper::getVersionNumber() {
	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) {
		return 0;
	}

	// allocate a block of memory for the version info
	DWORD dummy;
	UINT dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	if (dwSize == 0) {
		return 0;
	}
	std::vector<BYTE> data(dwSize);

	// load the version info
	if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0])) {
		return 0;
	}

	LPBYTE lpBuffer = NULL;

	if (VerQueryValue(&data[0], _T("\\"), (VOID FAR * FAR*) & lpBuffer, &dwSize)) {
		if (dwSize) {
			VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
			if (verInfo->dwSignature == 0xfeef04bd) {
				return ((verInfo->dwFileVersionMS >> 16) & 0xffff) |
					(((verInfo->dwFileVersionMS >> 0) & 0xffff) << 8) |
					(((verInfo->dwFileVersionLS >> 16) & 0xffff) << 16);
			}
		}
	}

	return 0;
}

wstring OpenKeyHelper::getVersionString() {
	TCHAR versionBuffer[MAX_PATH];
	DWORD ver = getVersionNumber();
	wsprintfW(versionBuffer, _T("%d.%d.%d"), ver & 0xFF, (ver>>8) & 0xFF, (ver >> 16) & 0xFF);
	return wstring(versionBuffer);

	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) { 
		return _T("");
	}
}

wstring OpenKeyHelper::getContentOfUrl(LPCTSTR url){
	//GetTempPath2 only exists on Windows 11 / Server 2022; importing it stops the exe from loading on Windows 10.
	//GetTempPath returns the path with a trailing backslash.
	WCHAR tempDir[MAX_PATH];
	DWORD tempDirLen = GetTempPathW(MAX_PATH, tempDir);
	if (tempDirLen == 0 || tempDirLen >= MAX_PATH) {
		return L"";
	}
	wstring path = wstring(tempDir) + L"_OpenKey.tempf";
	HRESULT res = URLDownloadToFile(NULL, url, path.c_str(), 0, NULL);

	if (res == S_OK) {
		std::wifstream t(path.c_str());
		std::wstringstream buffer;
		buffer << t.rdbuf();
		t.close();
		DeleteFile(path.c_str());
		return buffer.str();
	} else if (res == E_OUTOFMEMORY) {
		
	} else if (res == INET_E_DOWNLOAD_FAILURE) {
		
	} else {
		
	}
	return L"";
}
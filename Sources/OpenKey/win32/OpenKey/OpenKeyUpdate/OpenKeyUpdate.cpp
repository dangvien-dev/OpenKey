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

#include "framework.h"
#include "OpenKeyUpdate.h"
#include "../RepoConfig.h"
#include <Urlmon.h>
#include <shellapi.h>
#include <fstream>
#include <sstream>
#include <string>
#pragma comment(lib, "Urlmon.lib")

using namespace std;

INT_PTR CALLBACK MainDialogProcess(HWND, UINT, WPARAM, LPARAM);
void StartUpdate();
static void SetWorkingDirectoryToExeFolder();
static void DeleteOldUpdaterBackup();
HWND hDlg;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    SetWorkingDirectoryToExeFolder();
	DeleteOldUpdaterBackup();

	hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_UPDATER), 0, MainDialogProcess, 0);
	ShowWindow(hDlg, SW_SHOWNORMAL);
 
	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK MainDialogProcess(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:{
		HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_OPENKEYUPDATE));
		if (hIcon) {
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		}
		StartUpdate();
		return (INT_PTR)TRUE;
	}
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

//OpenKeyUpdate works with files relative to its own folder. OpenKey starts it with that folder as
//working directory, but make sure of it when it is started by hand or by an older OpenKey
//(which used its own working directory, e.g. C:\Windows\System32 when started at logon).
static void SetWorkingDirectoryToExeFolder() {
	WCHAR self[MAX_PATH];
	DWORD len = GetModuleFileNameW(NULL, self, MAX_PATH);
	if (len == 0 || len >= MAX_PATH) {
		return;
	}
	WCHAR* slash = wcsrchr(self, L'\\');
	if (slash != NULL) {
		*(slash + 1) = 0;
		SetCurrentDirectoryW(self);
	}
}

//Đường dẫn của chính OpenKeyUpdate.exe đang chạy (rỗng nếu không lấy được)
static wstring SelfPath() {
	WCHAR self[MAX_PATH];
	DWORD len = GetModuleFileNameW(NULL, self, MAX_PATH);
	if (len == 0 || len >= MAX_PATH) {
		return L"";
	}
	return wstring(self, len);
}

//Lần cập nhật trước đã đổi tên bản updater cũ thành *.old (không xóa được khi nó còn chạy): giờ dọn đi.
static void DeleteOldUpdaterBackup() {
	wstring self = SelfPath();
	if (!self.empty()) {
		DeleteFileW((self + L".old").c_str());
	}
}

//Thay chính OpenKeyUpdate.exe bằng bản mới trong gói. Windows không cho ghi đè tệp .exe đang chạy nhưng cho ĐỔI TÊN nó:
//đổi tên bản đang chạy thành *.old rồi chuyển bản mới vào đúng chỗ. Trước đây updater không bao giờ được cập nhật nên sửa lỗi
//ở updater không tới được máy người dùng. Chỉ là "cố gắng": thất bại thì giữ nguyên bản cũ, không ảnh hưởng việc cập nhật.
static void ReplaceSelfBestEffort() {
	wstring self = SelfPath();
	if (self.empty() || GetFileAttributes(L"_OpenKeyUpdate\\OpenKeyUpdate.exe") == INVALID_FILE_ATTRIBUTES) {
		return;
	}
	wstring backup = self + L".old";
	DeleteFileW(backup.c_str());
	if (!MoveFileExW(self.c_str(), backup.c_str(), MOVEFILE_REPLACE_EXISTING)) {
		return;
	}
	if (!MoveFileExW(L"_OpenKeyUpdate\\OpenKeyUpdate.exe", self.c_str(), MOVEFILE_COPY_ALLOWED)) {
		MoveFileExW(backup.c_str(), self.c_str(), MOVEFILE_REPLACE_EXISTING); //trả lại bản cũ
	}
}

//Mở lại TechVKey vừa cập nhật (cùng thư mục với updater). Thử CreateProcess trước; nếu Windows đòi nâng quyền
//(ERROR_ELEVATION_REQUIRED) hoặc lỗi khác thì để shell mở.
static bool RelaunchOpenKey(const wstring& dir) {
	wstring exe = dir + L"\\OpenKey64.exe";
	wstring cmd = L"\"" + exe + L"\"";
	STARTUPINFOW si = { sizeof(si) };
	PROCESS_INFORMATION pi = {};
	if (CreateProcessW(exe.c_str(), &cmd[0], NULL, NULL, FALSE, 0, NULL, dir.c_str(), &si, &pi)) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return true;
	}
	HINSTANCE r = ShellExecuteW(NULL, L"open", exe.c_str(), NULL, dir.c_str(), SW_SHOWNORMAL);
	return (INT_PTR)r > 32;
}

static void FailAndExit() {
	MessageBox(hDlg, _T("Có lỗi trong quá trình cập nhật, vui lòng thử lại sau!"), _T("TechVKey Update"), MB_OK);
	ExitProcess(0);
}

//The version name comes from a downloaded file and ends up in a URL and a file name: accept digits and dots only.
static bool IsSafeVersionName(const wstring& version) {
	if (version.empty() || version.size() > 31) {
		return false;
	}
	for (size_t i = 0; i < version.size(); i++) {
		if (!((version[i] >= L'0' && version[i] <= L'9') || version[i] == L'.')) {
			return false;
		}
	}
	return true;
}

//Run a hidden process and wait for it to finish (the old code started PowerShell and just slept 5 seconds).
static bool RunHiddenAndWait(const wstring& commandLine, DWORD timeoutMs) {
	wstring cmd = commandLine; //CreateProcessW may modify the buffer
	STARTUPINFOW si = { sizeof(si) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = {};
	if (!CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		return false;
	}
	bool finished = WaitForSingleObject(pi.hProcess, timeoutMs) == WAIT_OBJECT_0;
	if (!finished) {
		TerminateProcess(pi.hProcess, 1);
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return finished;
}

static void CleanUpUpdateFiles(const wstring& zipPath) {
	DeleteFile(zipPath.c_str());
	DeleteFile(L"_OpenKeyUpdate\\OpenKeyUpdate.exe");
	DeleteFile(L"_OpenKeyUpdate\\OpenKey64.exe");
	DeleteFile(L"_OpenKeyUpdate\\OpenKey32.exe");
	RemoveDirectory(L".\\_OpenKeyUpdate");
}

DWORD WINAPI UpdateThreadFunction(LPVOID lpParam) {
	WCHAR currentDir[MAX_PATH];
	DWORD currentDirLen = GetCurrentDirectory(MAX_PATH, currentDir);
	if (currentDirLen == 0 || currentDirLen >= MAX_PATH) {
		FailAndExit();
	}
	wstring path = wstring(currentDir) + L"\\_OpenKey.tempf";
	HRESULT res = URLDownloadToFile(NULL, OPENKEY_VERSION_JSON_URL, path.c_str(), 0, NULL);

	wstring data;
	if (res == S_OK) {
		std::wifstream t(path.c_str());
		std::wstringstream buffer;
		buffer << t.rdbuf();
		t.close();
		DeleteFile(path.c_str());
		data = buffer.str();
	} else {
		FailAndExit();
	}

	//simple parse; every find() is checked, substr(npos) would throw and kill the updater
	size_t pos = data.find(L"latestWinVersion");
	if (pos == wstring::npos) FailAndExit();
	pos = data.find(L"\"versionName\":", pos);
	if (pos == wstring::npos) FailAndExit();
	pos = data.find(L'"', pos + 14); //opening quote of the value
	if (pos == wstring::npos) FailAndExit();
	size_t endPos = data.find(L'"', pos + 1);
	if (endPos == wstring::npos) FailAndExit();
	wstring versionName = data.substr(pos + 1, endPos - pos - 1);
	if (!IsSafeVersionName(versionName)) FailAndExit();

	//download zip file
	wstring updateUrl = OPENKEY_RELEASE_DOWNLOAD_URL + versionName +
		L"/OpenKey" + versionName + L"-Windows.zip";
	wstring zipPath = wstring(currentDir) + L"\\_OpenKeyUpdate.zip";
	res = URLDownloadToFile(NULL, updateUrl.c_str(), zipPath.c_str(), 0, NULL);
	if (res != S_OK) {
		FailAndExit();
	}

	//extract zip file and wait for it; -Force because a previous failed attempt may have left files behind
	RunHiddenAndWait(L"powershell.exe -NoP -NonI -Command \"Expand-Archive -Force '.\\_OpenKeyUpdate.zip' '.\\_OpenKeyUpdate'\"", 120000);

	//Only touch the installed exe once the new one is really there. The old code deleted OpenKey64.exe
	//first, so an extraction that was slow or failed (no PowerShell 5) left the user without OpenKey.
	if (GetFileAttributes(L"_OpenKeyUpdate\\OpenKey64.exe") == INVALID_FILE_ATTRIBUTES) {
		CleanUpUpdateFiles(zipPath);
		FailAndExit();
	}

	//OpenKey may still be shutting down (an exe cannot be replaced while it runs): retry for up to 10 seconds
	bool replaced = false;
	for (int attempt = 0; attempt < 20 && !replaced; attempt++) {
		replaced = MoveFileEx(L"_OpenKeyUpdate\\OpenKey64.exe", L"OpenKey64.exe",
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != FALSE;
		if (!replaced) {
			Sleep(500);
		}
	}
	if (replaced) {
		ReplaceSelfBestEffort(); //phải làm trước CleanUpUpdateFiles vì nó xóa bản updater mới vừa giải nén
	}
	CleanUpUpdateFiles(zipPath);
	if (!replaced) {
		FailAndExit();
	}

	MessageBox(hDlg, _T("Bạn đã cập nhật TechVKey bản mới nhất thành công!\nTechVKey sẽ tự mở lại."), _T("TechVKey Update"), MB_OK);
	if (!RelaunchOpenKey(wstring(currentDir))) {
		MessageBox(hDlg, _T("Chưa tự mở lại được TechVKey, bạn hãy mở lại bằng tay."), _T("TechVKey Update"), MB_OK);
	}
	ExitProcess(0);
	return 0;
}

void StartUpdate() {
	DWORD hThread;
	HANDLE t = CreateThread(
							NULL,                   // default security attributes
							0,                      // use default stack size  
							UpdateThreadFunction,       // thread function name
							0,          // argument to thread function 
							0,                      // use default creation flags 
							&hThread);   // returns the thread identifier 
}
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

/*
 * Repo GitHub mà bản OpenKey này dùng để:
 *  - đọc version.json và tải bản mới khi cập nhật (OpenKeyManager::checkUpdate, OpenKeyUpdate.exe),
 *  - mở các link "Mã nguồn", "Trang chủ", "Phiên bản mới".
 *
 * Đây là NƠI DUY NHẤT khai báo repo. Khi đổi repo, chỉ sửa OPENKEY_REPO.
 * File này dùng chung cho cả project OpenKey và OpenKeyUpdate.
 *
 * Lưu ý: cập nhật tự động đọc qua raw.githubusercontent.com và github.com/.../releases
 * mà không có token, nên repo PHẢI là Public thì cập nhật mới hoạt động.
 *
 * Quy ước phát hành bản mới (bản Windows):
 *  - Tạo release với tag = tên phiên bản (ví dụ 2.0.6) và đính kèm file OpenKey<tên phiên bản>-Windows.zip
 *    (ví dụ OpenKey2.0.6-Windows.zip), trong đó OpenKey64.exe nằm ở gốc file zip.
 *  - Sửa version.json ở nhánh master: "latestWinVersion" gồm versionName và versionCode
 *    (versionCode = major | minor << 8 | build << 16, ví dụ 2.0.6 -> 393218).
 */
#define OPENKEY_REPO                  L"dangvien-dev/TechVKey"

#define OPENKEY_REPO_URL              L"https://github.com/" OPENKEY_REPO
#define OPENKEY_RELEASES_URL          OPENKEY_REPO_URL L"/releases"
#define OPENKEY_RELEASE_DOWNLOAD_URL  OPENKEY_REPO_URL L"/releases/download/"
#define OPENKEY_VERSION_JSON_URL      L"https://raw.githubusercontent.com/" OPENKEY_REPO L"/master/version.json"

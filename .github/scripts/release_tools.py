#!/usr/bin/env python3
"""Công cụ phát hành TechVKey cho Windows. Được gọi từ .github/workflows/release.yml.

  release_tools.py bump <X.Y.Z>             ghi số phiên bản mới vào version.json và OpenKey.rc
  release_tools.py package <X.Y.Z> <outdir>  đóng gói OpenKey<X.Y.Z>-Windows.zip từ kết quả build

Quy ước (phải khớp với OpenKeyUpdate.exe và OpenKeyManager::checkUpdate):
  - versionCode = major | minor << 8 | build << 16 (mỗi số 0..255), giống OpenKeyHelper::getVersionNumber.
  - Tag của release = tên phiên bản (ví dụ 2.0.6, KHÔNG có chữ v) và file đính kèm là OpenKey<tên>-Windows.zip.
  - OpenKey64.exe phải nằm ở GỐC file zip (updater giải nén vào _OpenKeyUpdate rồi lấy OpenKey64.exe).
"""
import re
import sys
import zipfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
WIN_DIR = Path('Sources/OpenKey/win32/OpenKey')
RC_FILE = WIN_DIR / 'OpenKey/OpenKey.rc'
VERSION_JSON = Path('version.json')

# (đường dẫn trong thư mục build) -> (tên trong file zip)
PACKAGE_FILES = [
    (WIN_DIR / 'x64/Release/OpenKey64.exe', 'OpenKey64.exe'),
    (WIN_DIR / 'x64/Release/OpenKeyUpdate.exe', 'OpenKeyUpdate.exe'),
    (WIN_DIR / 'Release/OpenKey32.exe', 'OpenKey32.exe'),
]


class ReleaseError(Exception):
    pass


def parse_version(text):
    # chặt chẽ: không khoảng trắng, không số 0 đứng đầu (02.0.6), để tag release và tên file zip luôn trùng chuỗi nhập vào
    m = re.fullmatch(r'(0|[1-9]\d{0,2})\.(0|[1-9]\d{0,2})\.(0|[1-9]\d{0,2})', text)
    if not m:
        raise ReleaseError('Số phiên bản phải có dạng X.Y.Z (ví dụ 2.0.6), không có số 0 đứng đầu, nhận được: %r' % text)
    parts = tuple(int(x) for x in m.groups())
    if any(p > 255 for p in parts):
        raise ReleaseError('Mỗi thành phần của phiên bản chỉ được từ 0 đến 255 (versionCode dùng 1 byte cho mỗi số).')
    return parts


def version_code(parts):
    major, minor, build = parts
    return major | (minor << 8) | (build << 16)


# "latestWinVersion": { "versionName": "2.0.2", "versionCode": 131074 }  (giữ nguyên định dạng của file)
WIN_BLOCK = re.compile(
    r'("latestWinVersion"\s*:\s*\{\s*"versionName"\s*:\s*")([^"]*)("\s*,\s*"versionCode"\s*:\s*)(\d+)')


def current_win_version(version_json_text):
    m = WIN_BLOCK.search(version_json_text)
    if not m:
        raise ReleaseError('Không tìm thấy khối "latestWinVersion" trong version.json.')
    return parse_version(m.group(2))


def bump_version_json(text, version, code):
    new_text, n = WIN_BLOCK.subn(lambda m: m.group(1) + version + m.group(3) + str(code), text, count=1)
    if n != 1:
        raise ReleaseError('Không cập nhật được version.json.')
    return new_text


def bump_rc(text, version):
    a, b, c = (int(x) for x in version.split('.'))
    subs = [
        (r'(?m)^( FILEVERSION )\d+,\d+,\d+,\d+$', r'\g<1>%d,%d,%d,0' % (a, b, c)),
        (r'(?m)^( PRODUCTVERSION )\d+,\d+,\d+,\d+$', r'\g<1>%d,%d,%d,0' % (a, b, c)),
        (r'(VALUE "FileVersion", ")[\d.]+(")', r'\g<1>%d.%d.%d.0\g<2>' % (a, b, c)),
        (r'(VALUE "ProductVersion", ")[\d.]+(")', r'\g<1>%d.%d.%d.0\g<2>' % (a, b, c)),
    ]
    for pattern, repl in subs:
        text, n = re.subn(pattern, repl, text)
        if n != 1:
            raise ReleaseError('Không tìm thấy đúng 1 dòng khớp %r trong OpenKey.rc (tìm thấy %d).' % (pattern, n))
    return text


def cmd_bump(version, root=REPO_ROOT):
    parts = parse_version(version)
    version = '%d.%d.%d' % parts
    code = version_code(parts)

    vj_path = root / VERSION_JSON
    vj_text = vj_path.read_text(encoding='utf-8')
    current = current_win_version(vj_text)
    if parts <= current:
        raise ReleaseError('Phiên bản mới %s phải lớn hơn phiên bản đang công bố trong version.json (%d.%d.%d).'
                           % ((version,) + current))

    vj_path.write_text(bump_version_json(vj_text, version, code), encoding='utf-8', newline='')
    rc_path = root / RC_FILE
    rc_path.write_text(bump_rc(rc_path.read_text(encoding='utf-8'), version), encoding='utf-8', newline='')
    print('Đã ghi phiên bản %s (versionCode %d) vào version.json và OpenKey.rc' % (version, code))
    return code


def cmd_package(version, out_dir, root=REPO_ROOT):
    parts = parse_version(version)
    version = '%d.%d.%d' % parts
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    zip_path = out_dir / ('OpenKey%s-Windows.zip' % version)

    for src, _ in PACKAGE_FILES:
        f = root / src
        if not f.is_file() or f.stat().st_size == 0:
            raise ReleaseError('Thiếu file build: %s' % src)
        with f.open('rb') as fh:
            if fh.read(2) != b'MZ':
                raise ReleaseError('%s không phải file thực thi Windows.' % src)

    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as z:
        for src, name in PACKAGE_FILES:
            z.write(root / src, arcname=name)  # arcname không có thư mục: file nằm ở gốc zip
    print(zip_path)
    return zip_path


def main(argv):
    try:
        if len(argv) == 3 and argv[1] == 'bump':
            cmd_bump(argv[2])
        elif len(argv) == 4 and argv[1] == 'package':
            cmd_package(argv[2], argv[3])
        else:
            print(__doc__)
            return 2
    except ReleaseError as e:
        print('LỖI: %s' % e, file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))

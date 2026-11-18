"""Chặn lỗi hỏng dấu tiếng Việt trong chuỗi của bản Windows.

Project Windows không dùng cờ /utf-8, nên MSVC chỉ hiểu tệp nguồn là UTF-8 khi tệp có BOM. Tệp không BOM bị đọc theo
bảng mã ANSI: chữ có dấu trong chuỗi (L"Không tìm thấy...") biến thành rác ("KhÃ´ng tÃ¬m tháº¥y..."), còn trình biên dịch
vẫn báo thành công. Quy ước của repo: tệp .cpp/.h nào có ký tự không phải ASCII trong CODE hoặc CHUỖI thì phải có BOM
(ký tự non-ASCII trong comment thì không sao).

Chạy: python -m unittest discover -s .github/scripts -v
"""
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
WIN_SRC = REPO / 'Sources/OpenKey/win32/OpenKey'
BOM = b'\xef\xbb\xbf'


def first_non_ascii_in_code(text):
    """Trả về số dòng (từ 1) của ký tự non-ASCII đầu tiên nằm ngoài comment, hoặc None."""
    state, i, line, n = 'code', 0, 1, len(text)
    while i < n:
        c = text[i]
        if c == '\n':
            line += 1
        if state == 'code':
            if text.startswith('//', i):
                state, i = 'line', i + 2
                continue
            if text.startswith('/*', i):
                state, i = 'block', i + 2
                continue
            if c == '"':
                state = 'str'
            elif c == "'":
                state = 'chr'
            elif ord(c) > 127:
                return line
        elif state == 'line':
            if c == '\n':
                state = 'code'
        elif state == 'block':
            if text.startswith('*/', i):
                state, i = 'code', i + 2
                continue
        else:  # 'str' hoặc 'chr'
            if c == '\\':
                i += 2
                continue
            if (state == 'str' and c == '"') or (state == 'chr' and c == "'") or c == '\n':
                state = 'code'
            elif ord(c) > 127:
                return line
        i += 1
    return None


def source_files():
    for sub in ('OpenKey', 'OpenKeyUpdate'):
        for p in sorted((WIN_SRC / sub).glob('*')):
            if p.suffix in ('.cpp', '.h'):
                yield p
    yield WIN_SRC / 'RepoConfig.h'


class ScannerTests(unittest.TestCase):
    def test_comments_are_ignored_but_strings_are_not(self):
        self.assertIsNone(first_non_ascii_in_code('int a; // tiếng Việt\n/* chú thích\ndài */ int b;'))
        self.assertEqual(first_non_ascii_in_code('int a;\nconst wchar_t* s = L"Không";'), 2)
        self.assertEqual(first_non_ascii_in_code('x = "http://a/b"; y = L"é";'), 1)  # "//" trong chuỗi không phải comment
        self.assertEqual(first_non_ascii_in_code('x = "a\\"é";'), 1)  # dấu \" không đóng chuỗi

    def test_the_bug_this_guards_against_would_be_caught(self):
        # đúng nội dung đã gây lỗi: chuỗi tiếng Việt trong tệp không BOM
        bug = 'msg = L"Windows không cho chạy:\\n" + path;'
        self.assertIsNotNone(first_non_ascii_in_code(bug))


class RepoSourceTests(unittest.TestCase):
    def test_every_file_with_non_ascii_code_has_a_bom(self):
        bad = []
        for p in source_files():
            raw = p.read_bytes()
            line = first_non_ascii_in_code(raw.decode('utf-8-sig'))
            if line is not None and not raw.startswith(BOM):
                bad.append('%s (dòng %d)' % (p.relative_to(REPO), line))
        self.assertEqual(bad, [], 'Thiếu BOM UTF-8, chữ tiếng Việt trong chuỗi sẽ bị hỏng dấu khi build:\n  ' + '\n  '.join(bad))


if __name__ == '__main__':
    unittest.main()

"""Kiểm thử release_tools.py. Chạy: python -m unittest discover -s .github/scripts -v"""
import shutil
import tempfile
import unittest
import zipfile
from pathlib import Path

import release_tools as rt

REPO = Path(__file__).resolve().parents[2]


def make_root():
    """Bản sao version.json và OpenKey.rc thật vào thư mục tạm."""
    root = Path(tempfile.mkdtemp())
    (root / rt.RC_FILE).parent.mkdir(parents=True)
    shutil.copy(REPO / rt.VERSION_JSON, root / rt.VERSION_JSON)
    shutil.copy(REPO / rt.RC_FILE, root / rt.RC_FILE)
    return root


class BumpTests(unittest.TestCase):
    def setUp(self):
        self.root = make_root()
        self.addCleanup(shutil.rmtree, self.root, ignore_errors=True)

    def read(self, rel):
        return (self.root / rel).read_text(encoding='utf-8')

    def test_version_code_layout_matches_the_app(self):
        # OpenKeyHelper::getVersionNumber: major | minor << 8 | build << 16
        self.assertEqual(rt.version_code((2, 0, 2)), 131074)  # giá trị có sẵn trong version.json gốc
        self.assertEqual(rt.version_code((2, 0, 6)), 393218)
        self.assertEqual(rt.version_code((2, 1, 0)), 258)

    def test_bump_writes_json_and_rc(self):
        macos_before = '"versionName": "2.0.3"'
        code = rt.cmd_bump('2.0.6', self.root)
        self.assertEqual(code, 393218)
        js = self.read(rt.VERSION_JSON)
        self.assertIn('"versionName": "2.0.6"', js)
        self.assertIn('"versionCode": 393218', js)
        self.assertIn(macos_before, js)  # mục macOS không bị đụng tới
        rc = self.read(rt.RC_FILE)
        for line in (' FILEVERSION 2,0,6,0', ' PRODUCTVERSION 2,0,6,0',
                     'VALUE "FileVersion", "2.0.6.0"', 'VALUE "ProductVersion", "2.0.6.0"'):
            self.assertIn(line, rc)

    def test_bump_rejects_bad_input_and_leaves_files_alone(self):
        before = (self.read(rt.VERSION_JSON), self.read(rt.RC_FILE))
        current = '%d.%d.%d' % rt.current_win_version(before[0])
        for bad in (current, '0.0.1', '2.0', '2.0.256', '2.0.6; rm -rf /', 'v2.0.7', '', '02.0.6', ' 2.0.7', '2.0.7\n'):
            with self.subTest(version=bad):
                with self.assertRaises(rt.ReleaseError):
                    rt.cmd_bump(bad, self.root)
        self.assertEqual(before, (self.read(rt.VERSION_JSON), self.read(rt.RC_FILE)))

    def test_cannot_release_the_same_version_twice(self):
        rt.cmd_bump('2.0.6', self.root)
        with self.assertRaises(rt.ReleaseError):
            rt.cmd_bump('2.0.6', self.root)


class PackageTests(unittest.TestCase):
    def setUp(self):
        self.root = Path(tempfile.mkdtemp())
        self.addCleanup(shutil.rmtree, self.root, ignore_errors=True)
        for src, _ in rt.PACKAGE_FILES:
            f = self.root / src
            f.parent.mkdir(parents=True, exist_ok=True)
            f.write_bytes(b'MZ' + b'\x00' * 64)

    def test_zip_name_and_layout_match_the_updater(self):
        zp = rt.cmd_package('2.0.6', self.root / 'dist', self.root)
        self.assertEqual(zp.name, 'OpenKey2.0.6-Windows.zip')  # OpenKeyUpdate.exe tải đúng tên này
        self.assertEqual(sorted(zipfile.ZipFile(zp).namelist()), ['OpenKey32.exe', 'OpenKey64.exe', 'OpenKeyUpdate.exe'])

    def test_rejects_missing_or_non_executable_output(self):
        (self.root / rt.PACKAGE_FILES[0][0]).write_bytes(b'not an exe')
        with self.assertRaises(rt.ReleaseError):
            rt.cmd_package('2.0.6', self.root / 'dist', self.root)
        (self.root / rt.PACKAGE_FILES[0][0]).unlink()
        with self.assertRaises(rt.ReleaseError):
            rt.cmd_package('2.0.6', self.root / 'dist', self.root)


if __name__ == '__main__':
    unittest.main()

# Bộ icon TechVKey (vẽ bằng vector)

Tạo 5 tệp `.ico` của bản Windows: `icon.ico` (keycap V), `StatusViet.ico`, `StatusEng.ico` (chữ trắng viền xanh),
`StatusViet10.ico`, `StatusEng10.ico` (chữ xanh đậm viền cyan). Các tệp `.ico` **đã được commit** trong
`Sources/OpenKey/win32/OpenKey/OpenKey/`; dựng ứng dụng (MSBuild) **không cần** Python hay thư mục này.
Chỉ chạy khi muốn chỉnh icon.

## Vì sao vẽ bằng vector

Bộ icon đầu tiên được cắt từ một ảnh render có nền caro, nên pixel ở mép dính viền xám/trắng (hiện rõ trên taskbar tối).
Khử viền bằng cách xử lý pixel không sạch được hẳn, nên các hình được **vẽ lại hoàn toàn** bằng Skia theo
`approved_design_source.png`: chữ là đa giác có đỉnh bo góc, nét cyan là hình lưỡi liềm giữa hai đường cong,
màu là gradient lấy mẫu từ ảnh gốc. Không còn pixel nào từ ảnh raster, nền trong suốt thật.

## Cách dùng

```bash
pip install -r tools/icons/requirements.txt

python tools/icons/build_icons.py                   # ghi đè 5 .ico trong thư mục icon của project Windows
python tools/icons/build_icons.py --out thu_muc     # hoặc ghi ra thư mục khác để xem thử trước
python tools/icons/preview.py thu_muc xem_thu       # ảnh xem thử: nền trắng/đen + cỡ khay trên taskbar sáng/tối
python tools/icons/build_icons.py --check thu_muc   # chỉ kiểm tra .ico (không cần skia)
```

`build_icons.py` luôn tự kiểm tra kết quả sau khi dựng: đủ 9 kích thước (16, 20, 24, 32, 40, 48, 64, 128, 256), góc ảnh
trong suốt, và dải 1-2px sát mép ngoài **không có pixel gần trắng** (bộ cũ dính viền ở đây: 56-77%, bộ hiện tại 0%).
Trả về mã thoát khác 0 nếu có vấn đề. Với cùng phiên bản `skia-python` (đã thử 144.1), chạy lại cho ra tệp `.ico` giống hệt từng byte với bộ đã commit.

Nên xem ảnh xem thử trên nền trắng, nền đen và cỡ khay trước khi commit icon mới.

## Chỉnh sửa

| Muốn đổi | Sửa ở |
|---|---|
| Hình chữ V/E, nét cyan | `status_icons.py`: `V_WHITE`, `E_WHITE`, `V_BLUE`, `E_BLUE` (đỉnh nhọn + bán kính bo + hai đường cong của nét) |
| Hình keycap | `keycap_icon.py` (`BASE`, `KEY`, `PLATEAU`, `GLYPH_V`, `HAT_*`, `SW_*`) |
| Màu | các dòng `d.gradient(..., [(vị trí, '#màu'), ...])` trong hai tệp trên |
| Độ dày nét ở từng kích thước | `OPTS` trong `build_icons.py` |

Toạ độ thiết kế là pixel của `approved_design_source.png` nhân 3 (keycap: ảnh cắt vùng phím rồi phóng 3 lần).
Các cỡ nhỏ 16-24px được chỉnh riêng (viền và nét cyan dày hơn, khe chữ E nới ra) vì ở cỡ khay hệ thống chữ chỉ cao
vài pixel; cỡ lớn giữ nguyên hình mẫu.

## Phạm vi

Chỉ 5 icon trên. Các icon nút (`OKButton`, `ExitButton`, `DefaultButton`, `StartConvert`) và icon của
`OpenKeyUpdate.exe` không do công cụ này tạo.

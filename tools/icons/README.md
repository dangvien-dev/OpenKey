# Công cụ vẽ bộ icon thay thế (vector)

> **Hiện chưa dùng.** Icon đang chạy trong `Sources/OpenKey/win32/OpenKey/OpenKey/` là **bộ gốc của tác giả OpenKey (2019)**.
> Bộ do công cụ này tạo (kiểu ChatGPT: chữ V/E nghiêng nét dày kèm nét cyan, keycap) đã được thử ở bản 2.0.8 rồi gỡ vì
> người dùng muốn quay lại icon gốc. Công cụ vẫn giữ lại để có thể tạo lại hoặc chỉnh. Mặc định nó ghi ra `tools/icons/out`
> (bị git bỏ qua) chứ **không** ghi đè icon của project; muốn thay icon thì chép tay các tệp `.ico` cần dùng.

Tạo 5 tệp `.ico`: `icon.ico` (keycap V), `StatusViet.ico`, `StatusEng.ico` (chữ trắng viền xanh),
`StatusViet10.ico`, `StatusEng10.ico` (chữ xanh đậm viền cyan). Dựng ứng dụng (MSBuild) **không cần** Python hay thư mục này.

## Vì sao vẽ bằng vector

Bộ icon đầu tiên được cắt từ một ảnh render có nền caro, nên pixel ở mép dính viền xám/trắng (hiện rõ trên taskbar tối).
Khử viền bằng cách xử lý pixel không sạch được hẳn, nên các hình được **vẽ lại hoàn toàn** bằng Skia theo
`approved_design_source.png`: chữ là đa giác có đỉnh bo góc, nét cyan là hình lưỡi liềm giữa hai đường cong,
màu là gradient lấy mẫu từ ảnh gốc. Không còn pixel nào từ ảnh raster, nền trong suốt thật.

## Cách dùng

```bash
pip install -r tools/icons/requirements.txt

python tools/icons/build_icons.py                   # ghi ra tools/icons/out
python tools/icons/build_icons.py --out thu_muc     # hoặc ghi ra thư mục khác
python tools/icons/preview.py [thu_muc] xem_thu     # ảnh xem thử: nền trắng/đen + cỡ khay trên taskbar sáng/tối
python tools/icons/build_icons.py --check thu_muc   # chỉ kiểm tra .ico (không cần skia)
```

`build_icons.py` luôn tự kiểm tra kết quả sau khi dựng: đủ 9 kích thước (16, 20, 24, 32, 40, 48, 64, 128, 256), góc ảnh
trong suốt, và dải 1-2px sát mép ngoài **không có pixel gần trắng** (bộ cũ dính viền ở đây: 56-77%, bộ hiện tại 0%).
Trả về mã thoát khác 0 nếu có vấn đề. Với cùng phiên bản `skia-python` (đã thử 144.1), chạy lại cho ra tệp `.ico` giống hệt từng byte với bộ đã commit.

Nên xem ảnh xem thử trên nền trắng, nền đen và cỡ khay trước khi commit icon mới.

## Chỉnh sửa

| Muốn đổi | Sửa ở |
|---|---|
| Độ đậm chữ V/E | `status_icons.py`: `V_STEM`, `E_ARM`, `E_SPINE` (chữ do `regular_v()`/`regular_e()` dựng từ các số này) |
| Nét cyan | `status_icons.py`: `sw_outer`, `sw_inner` của `V_WHITE`, `E_WHITE`, `V_BLUE`, `E_BLUE` (hai đường cong từ mũi trái sang mũi phải) |
| Hình keycap | `keycap_icon.py` (`BASE`, `KEY`, `PLATEAU`, `GLYPH_V`, `HAT_*`, `SW_*`) |
| Màu | các dòng `d.gradient(..., [(vị trí, '#màu'), ...])` trong hai tệp trên |
| Độ dày nét ở từng kích thước | `OPTS` trong `build_icons.py` |

Chữ V/E dùng độ đậm **Regular dày nhẹ** (nét ~17-18% chiều cao chữ). Bản đầu cắt từ ảnh ChatGPT là dạng Black (nét ~29%) nên
trông nặng; muốn đậm hơn/nhạt hơn chỉ cần đổi `V_STEM`, `E_ARM`, `E_SPINE`. Khung ngoài, độ nghiêng, màu và nét cyan giữ như bản duyệt.

Toạ độ thiết kế là pixel của `approved_design_source.png` nhân 3 (keycap: ảnh cắt vùng phím rồi phóng 3 lần).
Mọi cỡ giữ **nét mảnh** như bản lớn. Đã thử làm đậm viền và nét cyan ở cỡ khay (16-24px) nhưng trông bè và lem nên
bỏ; cỡ nhỏ chỉ chỉnh riêng khe chữ E (nới ra, vì ở 16px khe chỉ còn dưới 1 pixel) và lề quanh biểu tượng.
`status_icons.render` còn hai tuỳ chọn thử nghiệm chưa dùng: `swoosh='none'|'flat'` (bỏ nét cyan / nét cyan đặc) và
`bright=True` (thân chữ xanh sáng hơn).

## Phạm vi

Chỉ 5 icon trên. Các icon nút (`OKButton`, `ExitButton`, `DefaultButton`, `StartConvert`) và icon của
`OpenKeyUpdate.exe` không do công cụ này tạo.

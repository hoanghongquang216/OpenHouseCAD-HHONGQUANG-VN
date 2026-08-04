# Decision Gate — svg-pipeline (formerly DS-004 / DS-005)

## Trạng thái: HOÀN TẤT

Commit cuối: `90296df` (main), sau `75bcaa8` (code test) và `c3248d6`/`f5353c5` (nền trước đó).

---

## Deliverable cuối cùng

- `modules/dxf/tests/SvgPipelineIntegrationTests.cpp` — 10 test case,
  kiểm chứng toàn bộ pipeline DXF → Document → SVG chạy xuyên suốt qua
  một `Document` thật, không chỉ từng tầng riêng lẻ.
- Đăng ký vào `modules/dxf/tests/CMakeLists.txt` dưới tên
  `OpenHouseSvgPipelineIntegrationTests`.
- `CHANGELOG.md` — mục `[svg-pipeline]`.
- `docs/ENGINEERING_PRINCIPLES.md` — Nguyên tắc 16 (mới).

## Bằng chứng hoàn tất

- Build sạch: `cmake --build build --parallel` không lỗi, không warning.
- **26/26 test pass** toàn bộ repo (`ctest --test-dir build`), bao gồm
  cả `OpenHouseSvgPipelineIntegrationTests` lẫn 25 test có sẵn từ
  trước — không phá vỡ test nào.
- Đã push lên `origin/main`, xác nhận qua `git log`/`git push`.

## Việc KHÔNG làm (và lý do)

- **Không tạo 16 file `.dxf` + 12 file `.svg` "vàng" trên đĩa** như
  thiết kế ban đầu — trùng lặp với `DxfReaderTests.cpp` đã có, và
  pattern "so khớp golden file" không tồn tại ở bất kỳ đâu khác trong
  codebase.
- **Không dùng GoogleTest** — codebase dùng `OH_CHECK` + `main()` (qua
  `OpenHouse::Testing`), không phải GTest.
- **Không sửa `DXF_BACKLOG.md`** — file này chỉ dành cho việc *chưa*
  làm; sprint đã shipped nên chỉ cần ghi ở `CHANGELOG.md`.
- **Không mở ticket `DS-005R`** — cùng 1 ticket, thiết kế được điều
  chỉnh trong quá trình implementation, ghi nhận bằng "Superseded
  during implementation" thay vì mở ticket mới.

---

## Bài học (đã ghi vào Nguyên tắc 16)

**Convention của codebase quan trọng hơn thiết kế ban đầu của ticket.**
Khi thiết kế đề ra ban đầu mâu thuẫn với convention đã được kiểm chứng
trong codebase, mặc định là điều chỉnh thiết kế cho khớp codebase —
trừ khi pattern mới có lợi ích cụ thể, đã chứng minh được mà convention
cũ không có.

## Bài học bổ sung (chưa ghi vào Nguyên tắc, nên cân nhắc thêm)

1. **Bịa thông tin chưa xác minh xảy ra 3 lần liên tiếp trong cùng 1
   sprint** — không phải lỗi ngẫu nhiên một lần:
   - Lần 1: bịa API `DxfReader::ParseDxfFile`/`SvgExporter::ExportSvg`
     không tồn tại trong codebase thật.
   - Lần 2: khẳng định `ENGINEERING_PRINCIPLES.md` có mục "Deferred
     Engineering Backlog" — không tồn tại.
   - Lần 3: mô tả chi tiết đầy đủ nội dung `DXF-ROBUST-001` (rationale,
     proposed behaviour...) trong khi bản gốc ghi rõ
     `CONTENT INCOMPLETE, needs follow-up`.

   → Khuyến nghị: bất kỳ khẳng định nào về nội dung file cụ thể (API,
   tài liệu, backlog...) cần được xác minh bằng cách đọc trực tiếp file
   thật (`grep`, `cat`, raw link) trước khi đưa vào quyết định kỹ
   thuật, không dựa trên suy luận "nghe hợp lý".

2. **Thao tác qua terminal nhiều dòng dễ gây lỗi** — heredoc không
   đóng (xảy ra 2 lần với script sinh sprint package), commit message
   nhiều dòng bị rơi vào `nano`, paste bị lẫn vào `less`/editor lồng
   nhau. Với thay đổi nhiều dòng, ghi ra file riêng rồi dùng
   `git commit -F <file>` hoặc ghi đè toàn bộ file đích, thay vì gõ/dán
   trực tiếp nhiều dòng vào lệnh terminal, đáng để thành thói quen mặc
   định — không chỉ khi đã gặp lỗi.

---

## Quyết định

- [x] Đóng DS-004 — ghi "Superseded during implementation" (không phải Rejected)
- [x] Đóng DS-005 — ghi "Delivered as SVG Pipeline Integration Tests"
- [x] Không mở DS-005R
- [x] CHANGELOG phản ánh deliverable thật, không phản ánh kế hoạch ban đầu
- [x] Backlog giữ nguyên 1 nguồn (`DXF_BACKLOG.md`, không đụng vào)

**Sprint tiếp theo:** chưa xác định — cần bạn/Deep quyết dựa trên
roadmap (`docs/ROADMAP_EXECUTION.md`), không nằm trong phạm vi Decision
Gate này.

# BÁO CÁO THỰC HÀNH
## Thiết lập pipeline CI/CD tự động với GitHub Actions

**Người thực hiện:** nv-hoai  
**Ngày thực hiện:** 19/10/2026  
**Repository:** https://github.com/nv-hoai/cicd (Public)  
**Môi trường cục bộ:** Fedora Linux 44 (WSL), GCC 16.1.1, GNU Make, Cppcheck 2.22.0  
**Môi trường CI:** GitHub Actions — `ubuntu-latest`

---

## 1. Mục tiêu

Xây dựng hoàn chỉnh một pipeline tự động hóa cho chương trình C kiểm tra số nguyên tố, bao gồm:

- **CI (Continuous Integration):** tự động **Lint → Build → Test** mỗi khi có push hoặc pull request vào nhánh `main`.
- **CD (Continuous Delivery/Deployment):** tự động build, đóng gói và phát hành GitHub Release khi push tag phiên bản `v*`.
- **Bảo vệ nhánh `main`:** bắt buộc status check CI phải xanh trước khi merge.

---

## 2. Kết quả tổng quan

| Hạng mục | Kết quả |
| --- | --- |
| Repository public | ✅ https://github.com/nv-hoai/cicd |
| Mã nguồn + Makefile | ✅ `prime_utils.c`, `test_app.c`, `Makefile` |
| Workflow CI | ✅ `.github/workflows/ci.yml` — 3 giai đoạn Lint/Build/Test |
| Workflow CD | ✅ `.github/workflows/release.yml` — phát hành khi có tag `v*` |
| Lần chạy CI gần nhất | ✅ Success — [run #39224052139](https://github.com/nv-hoai/cicd/actions/runs/39224052139) |
| Release | ✅ `v1.0.0` kèm asset `prime-validator-linux-amd64.tar.gz` |
| Bảo vệ nhánh `main` | ✅ Yêu cầu status check `continuous-integration` |

---

## 3. Các bước thực hiện

### 3.1. Chuẩn bị mã nguồn và bộ kiểm thử

Đã tạo các file theo hướng dẫn:

| File | Vai trò |
| --- | --- |
| `prime_utils.c` | Cài đặt hàm `is_prime(int n)` |
| `test_app.c` | Unit test bằng `assert()`, gồm cả edge case `INT_MAX = 2147483647` |
| `Makefile` | Định nghĩa 4 target: `build`, `test`, `lint`, `clean` |
| `.gitignore` | Bỏ qua `validator` và `*.tar.gz` |

Kiểm tra cục bộ trước khi đẩy lên GitHub:

```bash
make lint    # Cppcheck
make build   # GCC
make test    # chạy ./validator
```

Kết quả:

```text
gcc -Wall -Wextra -O3 prime_utils.c test_app.c -o validator
./validator
TẤT CẢ CÁC BÀI UNIT TEST ĐÃ VƯỢT QUA SẠCH LỖI!
```

---

### 3.2. Thiết lập pipeline CI

File `.github/workflows/ci.yml` được cấu hình:

- Kích hoạt khi **push** hoặc **pull_request** vào nhánh `main`.
- Job tên `continuous-integration` chạy trên `ubuntu-latest`.
- Cài `gcc`, `make`, `cppcheck`, sau đó lần lượt chạy:
  1. `make lint` — quét mã tĩnh.
  2. `make build` — biên dịch.
  3. `make test` — chạy unit test.

![Nội dung workflow CI](images/05-ci-yml.png)

*Hình 1: Workflow `ci.yml` với 3 giai đoạn Lint → Build → Test.*

> Ghi chú: dấu ❌ cạnh commit trong ảnh là trạng thái của commit đầu tiên từng làm CI thất bại ở bước lint. Sự cố này đã được xử lý và được trình bày ở mục 4.

---

### 3.3. Thiết lập pipeline CD — Release Automation

File `.github/workflows/release.yml` được cấu hình:

- Chỉ kích hoạt khi push tag bắt đầu bằng `v` (`v*`).
- Cấp quyền `contents: write` để tạo Release.
- Các bước: checkout toàn bộ lịch sử Git → cài GCC/Make → `make build` → nén `validator` thành `prime-validator-linux-amd64.tar.gz` → dùng `softprops/action-gh-release@v2` để tạo Release và đính kèm asset, tự sinh release notes.

---

### 3.4. Đẩy mã nguồn, tạo tag và phát hành

Các lệnh Git đã thực hiện:

```bash
git init -b main
git add .
git commit -m "ci/cd: complete core lint-build-test pipeline and release automation workflow"
gh repo create nv-hoai/cicd --public --source=. --remote=origin --push

git tag v1.0.0 -m "Phát hành phiên bản chính thức đầu tiên bản v1.0.0"
git push origin v1.0.0
```

Kết quả trên GitHub:

- **CI** chạy thành công với cả 3 bước màu xanh.
- **CD** được kích hoạt riêng khi tag `v1.0.0` được đẩy lên và cũng thành công.
- GitHub Release `v1.0.0` xuất hiện với file nén đính kèm.

![Trang chủ repository](images/01-repo-home.png)

*Hình 2: Repository public với đầy đủ mã nguồn, workflow và README.*

![Danh sách workflow chạy thành công](images/03-actions-success.png)

*Hình 3: Tab Actions (lọc `is:success`) — cả workflow CI và CD đều thành công.*

![Chi tiết lần chạy CI](images/02-ci-run.png)

*Hình 4: Lần chạy CI gần nhất — job `continuous-integration` kết thúc với trạng thái Success.*

![Trang GitHub Release v1.0.0](images/04-release.png)

*Hình 5: Release `v1.0.0` với asset `prime-validator-linux-amd64.tar.gz` (2.7 KB).*

Đã tải lại asset từ Release và kiểm tra nội dung:

```text
-rwxr-xr-xr-x runner/runner 16128 ... validator
asset OK
```

---

### 3.5. Thiết lập luật bảo vệ nhánh `main`

Đã bật branch protection cho nhánh `main` với các thiết lập:

| Thiết lập | Giá trị |
| --- | --- |
| Require status checks to pass | ✅ Bật |
| Status check bắt buộc | `continuous-integration` |
| Strict (branch phải cập nhật mới nhất) | ✅ Bật |
| Enforce cho admin | ❌ Tắt (chủ repo vẫn có thể push trực tiếp) |
| Cho phép force push | ❌ Tắt |
| Cho phép xóa nhánh | ❌ Tắt |

Cấu hình được áp dụng qua GitHub API:

```bash
gh api repos/nv-hoai/cicd/branches/main/protection -X PUT --input - <<'EOF'
{
  "required_status_checks": {
    "strict": true,
    "contexts": ["continuous-integration"]
  },
  "enforce_admins": false,
  "required_pull_request_reviews": null,
  "restrictions": null
}
EOF
```

![Trang Branches với biểu tượng khóa](images/06-branches.png)

*Hình 6: Nhánh `main` hiển thị biểu tượng khiên bảo vệ trên trang Branches.*

Từ thời điểm này, pull request có CI thất bại sẽ không thể merge vào `main` cho đến khi pipeline xanh trở lại.

---

## 4. Sự cố phát sinh và cách xử lý

### 4.1. Lần chạy CI đầu tiên thất bại ở bước Lint

Lần push đầu tiên, CI dừng ở giai đoạn Lint với lỗi:

```text
cppcheck --enable=all --error-exitcode=1 prime_utils.c test_app.c
prime_utils.c:1:0: information: Include file: <stdio.h> not found ... [missingIncludeSystem]
test_app.c:1:0: information: Include file: <stdio.h> not found ... [missingIncludeSystem]
test_app.c:2:0: information: Include file: <assert.h> not found ... [missingIncludeSystem]
nofile:0:0: information: Active checkers: ... [checkersReport]
make: *** [Makefile:14: lint] Error 1
```

**Nguyên nhân:** Cppcheck trên runner Ubuntu phát sinh các thông báo mức `information` về môi trường (không tìm thấy header chuẩn, báo cáo checker). Với tùy chọn `--error-exitcode=1`, các thông báo này khiến lệnh trả về mã lỗi, dù mã nguồn không có lỗi thực sự.

**Cách khắc phục:** Bổ sung hai suppression cho các thông báo chỉ mang tính môi trường, vẫn giữ `--enable=all` và `--error-exitcode=1` để bắt lỗi thật:

```makefile
lint:
	cppcheck --enable=all --suppress=missingIncludeSystem --suppress=checkersReport --error-exitcode=1 prime_utils.c test_app.c
```

Sau khi commit và push bản sửa, CI chuyển sang **Success** (Hình 3, Hình 4).

### 4.2. Ghi chú

Bài thực hành có phần mô phỏng cố ý làm hỏng hàm `is_prime` để thấy pipeline chuyển đỏ. Phần này **không được thực hiện** trên nhánh `main` nhằm giữ repository ở trạng thái xanh; có thể thực hiện lại bất cứ lúc nào trên một nhánh/PR riêng.

---

## 5. Kiểm chứng cuối cùng

| Kiểm tra | Cách thực hiện | Kết quả |
| --- | --- | --- |
| Lint cục bộ | `make lint` | ✅ Exit 0 |
| Build cục bộ | `make build` | ✅ Sinh file `validator` |
| Unit test cục bộ | `make test` | ✅ In thông báo vượt qua toàn bộ test |
| CI trên GitHub | Tab Actions | ✅ Success, 21 giây |
| CD trên GitHub | Push tag `v1.0.0` | ✅ Success, 15 giây |
| Release asset | Tải và giải nén `.tar.gz` | ✅ Chứa file `validator` |
| Branch protection | `gh api .../branches/main/protection` | ✅ Bắt buộc `continuous-integration` |

---

## 6. Kết luận

Pipeline CI/CD đã được thiết lập hoàn chỉnh và hoạt động đúng như yêu cầu:

- Mọi thay đổi đẩy lên `main` đều phải đi qua **Lint → Build → Test**.
- Việc phát hành phiên bản được tự động hóa hoàn toàn: chỉ cần push tag `v*`, GitHub Actions sẽ build, đóng gói và tạo Release kèm file tải về.
- Nhánh `main` được bảo vệ bởi status check `continuous-integration`, ngăn code lỗi được merge.
- Qua sự cố Cppcheck, đã hiểu rõ hơn cách phân biệt thông báo môi trường với lỗi mã nguồn thực sự trong pipeline.

**Repository:** https://github.com/nv-hoai/cicd  
**Release:** https://github.com/nv-hoai/cicd/releases/tag/v1.0.0

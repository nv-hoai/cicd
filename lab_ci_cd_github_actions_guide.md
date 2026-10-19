# THỰC HÀNH: THIẾT LẬP PIPELINE TỰ ĐỘNG HÓA CI/CD VỚI GITHUB ACTIONS

**Môi trường yêu cầu:** Máy tính cài sẵn Git, tài khoản GitHub, trình biên dịch GCC/Make (hoặc giả lập trên môi trường Linux Ubuntu).  
**Mục tiêu:** Tự tay thiết lập cấu trúc mã nguồn, viết file cấu hình YAML hoàn chỉnh để tự động hóa 3 giai đoạn: **Linter (Quét định dạng) ➔ Build (Biên dịch) ➔ Test (Kiểm thử)**. Xây dựng pipeline CD với **Release Automation**.

---

## HƯỚNG DẪN CHI TIẾT

### Bước 1: Chuẩn bị phôi dự án và Bộ kiểm thử

> **Yêu cầu:** Có sẵn code và file chạy lệnh.

Để robot trên đám mây biết phải làm gì, bạn cần cung cấp cho nó mã nguồn và câu lệnh biên dịch tiêu chuẩn. Tạo một thư mục dự án cục bộ chứa 3 file sau:

#### File 1: `prime_utils.c` (Mã nguồn thuật toán lõi)

```c
#include <stdio.h>

/**
 * @brief Kiểm tra một số có phải là số nguyên tố hay không.
 */
int is_prime(int n) {
    if (n <= 1) return 0;
    
    for (int i = 2; i <= n / i; i++) {
        if (n % i == 0) return 0; 
    }
    return 1;
}
```

#### File 2: `test_app.c` (Hệ thống chạy Unit Test)

```c
#include <stdio.h>
#include <assert.h>

// Khai báo prototype hàm từ file prime_utils.c
int is_prime(int n);

int main() {
    // 1. Kiểm tra các kịch bản cơ bản
    assert(is_prime(1) == 0); // Số 1 không phải số nguyên tố
    assert(is_prime(2) == 1); // Số 2 là số nguyên tố
    assert(is_prime(3) == 1); // Số 3 là số nguyên tố
    assert(is_prime(4) == 0); // Số 4 không phải số nguyên tố
    assert(is_prime(5) == 1); // Số 5 là số nguyên tố

    // 2. Kiểm tra biên dữ liệu lớn để test chốt chặn chống tràn số (Edge Case)
    // 2147483647 chính là INT_MAX và là một số nguyên tố danh tiếng
    assert(is_prime(2147483647) == 1); 
    
    printf("TẤT CẢ CÁC BÀI UNIT TEST ĐÃ VƯỢT QUA SẠCH LỖI!\n");
    return 0;
}
```

#### File 3: `Makefile` (File đóng gói câu lệnh biên dịch)

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O3

# 1. Giai đoạn Biên dịch (Build)
build:
	$(CC) $(CFLAGS) prime_utils.c test_app.c -o validator

# 2. Giai đoạn Kiểm thử (Test)
test: build
	./validator

# 3. Giai đoạn Quét mã tĩnh (Lint) - Bắt lỗi phong cách và bug ngầm
lint:
	cppcheck --enable=all --suppress=missingIncludeSystem --suppress=checkersReport --error-exitcode=1 prime_utils.c test_app.c
	# Giải thích: --error-exitcode=1 ép cppcheck phải ném ra lỗi làm dừng sập pipeline nếu phát hiện code xấu.

# 4. Dọn dẹp hệ thống
clean:
	rm -f validator *.tar.gz
```

> 💡 **Mẹo:** Hãy chạy thử lệnh `make test` ở máy cá nhân để đảm bảo mọi thứ hiện chữ xanh thành công trước khi đẩy lên đám mây.

---

### Bước 2: Thiết lập pipeline Tự động hóa `.github/workflows/ci.yml`

1. Tạo một cấu trúc thư mục ẩn chính xác như sau tại thư mục gốc của dự án: `.github/workflows/`.
2. Bên trong thư mục này, tạo file cấu hình luồng chạy tên là `ci.yml`.

Sao chép đoạn mã cấu hình chuẩn dưới đây. Đây chính là "bản thiết kế" ra lệnh cho máy chủ ảo Ubuntu của GitHub tự động vận hành:

```yaml
name: Continuous Integration (CI) Quality Gate

on:
  push:
    branches: [ "main" ]
  pull_request:
    branches: [ "main" ]

jobs:
  continuous-integration:
    runs-on: ubuntu-latest
    steps:
    - name: Checkout Source Code
      uses: actions/checkout@v4

    # Cài đặt môi trường bao gồm cả trình biên dịch và công cụ quét tĩnh Cppcheck
    - name: Install Toolchains (GCC & Cppcheck)
      run: |
        sudo apt-get update
        sudo apt-get install -y gcc make cppcheck

    # GIAI ĐOẠN 1: LINT (Quét lỗi tĩnh)
    - name: Run Stage 1 - Static Code Analysis (Lint)
      run: make lint

    # GIAI ĐOẠN 2: BUILD (Biên dịch)
    - name: Run Stage 2 - Compile and Build
      run: make build

    # GIAI ĐOẠN 3: TEST (Chạy Unit Test)
    - name: Run Stage 3 - Execute Unit Tests
      run: make test
```

---

### Bước 3: Xây dựng pipeline CD — Release Automation

Bây giờ, chúng ta muốn tạo một pipeline riêng cho việc phát hành sản phẩm. Mỗi khi chúng ta gắn thẻ phiên bản mới (Ví dụ: `v1.2.0`), hệ thống sẽ tự động đóng gói file chạy `validator` thành file nén `.tar.gz` và tạo trang tải về trên GitHub.

Tạo thêm một file cấu hình mới nằm tại đường dẫn: `.github/workflows/release.yml`

```yaml
name: Automated Production Release (CD)

# Chỉ kích hoạt đường ống này khi có sự kiện đẩy một Git Tag bắt đầu bằng chữ 'v'
on:
  push:
    tags:
      - 'v*'

jobs:
  publish-artifacts:
    runs-on: ubuntu-latest
    
    # Cấp quyền cho robot được phép ghi dữ liệu và đăng tải file lên mục Release của Repo
    permissions:
      contents: write

    steps:
    - name: Checkout Code
      uses: actions/checkout@v4
      with:
        fetch-depth: 0 # Tải toàn bộ lịch sử Git để robot tự viết Nhật ký thay đổi (Changelog)

    - name: Install Compilers
      run: sudo apt-get install -y gcc make

    # Biên dịch bản Production chính thức
    - name: Build Production Binary
      run: make build

    # Đóng gói file chạy thành định dạng nén tiêu chuẩn của Linux (.tar.gz)
    - name: Package Artifact Archive
      run: |
        tar -czvf prime-validator-linux-amd64.tar.gz validator

    # SỬ DỤNG ROBOT TỰ ĐỘNG XUẤT BẢN RELEASE
    - name: Deploy Asset to GitHub Releases
      uses: softprops/action-gh-release@v2
      with:
        # Đính kèm file nén vừa đóng gói lên mạng cho người dùng tải về
        files: prime-validator-linux-amd64.tar.gz
        
        # Bật tính năng tự động quét các Pull Request để tự viết bản tin Release Notes sạch đẹp
        generate_release_notes: true
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }} # Chìa khóa bảo mật tự động của GitHub cấp cho máy ảo
```

---

### Bước 4: Nghiệm thu với Git Tag

Mở Terminal cục bộ trên máy bạn và chạy chuỗi lệnh sau:

1. **Đẩy file cấu hình mới lên kho lưu trữ:**
   ```bash
   git add .
   git commit -m "ci/cd: complete core lint-build-test pipeline and release automation workflow"
   git push origin main
   ```

2. **KÍCH HOẠT LỆNH PHÁT HÀNH PHIÊN BẢN (CD):**  
   Gõ 2 câu lệnh Git để gắn thẻ tag phiên bản `v1.0.0` và đẩy lên đám mây:
   ```bash
   git tag v1.0.0 -m "Phát hành phiên bản chính thức đầu tiên bản v1.0.0"
   git push origin v1.0.0
   ```

3. **Kiểm tra kết quả trên Web GitHub:**
   - Vào tab **Actions**, bạn sẽ thấy workflow `Automated Production Release (CD)` được kích hoạt riêng biệt.
   - Chờ khoảng 30 giây, tiến trình sẽ chuyển sang **Màu xanh lá cây (Success)**. Bấm vào để đọc nhật ký in ra dòng chữ `TẤT CẢ CÁC BÀI UNIT TEST ĐÃ VƯỢT QUA SẠCH LỖI!`.

4. **MÔ PHỎNG SẬP PIPELINE:**  
   - Để trải nghiệm sức mạnh của CI, hãy mở file `prime_utils.c` ra sửa lỗi logic nhằm làm lỗi bài test: sửa hàm `is_prime` cho trả về giá trị `1` khi `n = 4`.
   - Tiến hành commit và push lên lại. Quay lại tab **Actions**, bạn sẽ thấy tiến trình chuyển sang **Màu đỏ dấu X (Failed)**. Hệ thống sẽ gửi email cảnh báo cho toàn bộ thành viên trong nhóm.
   - Hãy sửa lại file `prime_utils.c` cho đúng chuẩn kỹ thuật và push lên để đưa pipeline trở lại trạng thái màu xanh an toàn.

5. **Xác nhận Release:**  
   - Sau khi hiện tích xanh, bạn quay trở lại trang chủ của Repository (Tab **Code**).
   - Nhìn sang góc bên phải, tại mục **Releases**, bạn sẽ thấy một phiên bản mới tinh xuất hiện có tên là **Release v1.0.0**.
   - Mở trang Release ra, bạn sẽ thấy robot đã tự động viết danh sách các commit và đính kèm sẵn file nén `prime-validator-linux-amd64.tar.gz` sạch đẹp cho phép tải về. Quy trình tự động hóa hoàn tất!

---

### Bước 5: Thiết lập Luật bảo vệ nhánh chính (Branch Protection Rules)

Để ngăn chặn tình trạng một thành viên trong nhóm "vô ý" merge một đoạn code đang bị lỗi đỏ vào nhánh `main`, bạn cần khóa nhánh `main` lại bằng luật của GitHub:

1. Truy cập vào **Settings** của repository trên GitHub.
2. Ở thanh menu bên trái, chọn mục **Branches** ➔ Bấm vào nút **Add branch ruleset** (hoặc **Add branch protection rule**).
3. Tại ô **Target branches**, đặt điều kiện áp dụng cho nhánh `main`.
4. Tích chọn vào mục: **"Require status checks to pass before merging"** (Bắt buộc các bài kiểm tra trạng thái phải vượt qua mới được gộp code).
5. Tại thanh tìm kiếm hiện ra, gõ chữ `continuous-integration` (chính là tên job chúng ta đã định nghĩa trong file YAML) và bấm chọn nó làm chốt chặn đóng dấu an toàn.
6. Bấm **Save changes**.

Từ giây phút này, nếu code của một thành viên gửi lên không vượt qua được bài kiểm thử tự động, nút **Merge Pull Request** sẽ bị xám màu và khóa chặt, không một ai có quyền gộp mã lỗi vào hệ thống.
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
	cppcheck --enable=all --error-exitcode=1 prime_utils.c test_app.c
	# Giải thích: --error-exitcode=1 ép cppcheck phải ném ra lỗi làm dừng sập pipeline nếu phát hiện code xấu.

# 4. Dọn dẹp hệ thống
clean:
	rm -f validator *.tar.gz

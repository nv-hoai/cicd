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

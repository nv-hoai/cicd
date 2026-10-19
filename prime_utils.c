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

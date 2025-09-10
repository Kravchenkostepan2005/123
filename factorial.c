#include <stdio.h>
#include <limits.h>

// Вычисляет n! для 0 <= n <= 20 (вмещается в unsigned long long)
static unsigned long long factorial(unsigned int n) {
    unsigned long long result = 1ULL;
    for (unsigned int i = 2; i <= n; ++i) {
        result *= (unsigned long long)i;
    }
    return result;
}

int main(void) {
    long long input;
    printf("Введите целое число от 0 до 20: ");
    if (scanf("%lld", &input) != 1) {
        fprintf(stderr, "Ошибка: не удалось прочитать число.\n");
        return 1;
    }

    if (input < 0) {
        fprintf(stderr, "Ошибка: факториал определён только для неотрицательных чисел.\n");
        return 1;
    }
    if (input > 20) {
        fprintf(stderr, "Ошибка: 21! и больше не помещается в 64-битный тип.\n");
        return 1;
    }

    unsigned int n = (unsigned int)input;
    unsigned long long value = factorial(n);
    printf("%u! = %llu\n", n, value);
    return 0;
}


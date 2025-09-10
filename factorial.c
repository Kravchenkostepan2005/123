// Simple factorial program in C (iterative), supports input via argv or stdin
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static int read_non_negative_input(int argc, char **argv, unsigned long long *out_value) {
	if (out_value == NULL) {
		return -1;
	}

	if (argc >= 2) {
		char *end_ptr = NULL;
		errno = 0;
		long long parsed = strtoll(argv[1], &end_ptr, 10);
		if (errno != 0 || end_ptr == argv[1] || (end_ptr != NULL && *end_ptr != '\0') || parsed < 0) {
			return -1;
		}
		*out_value = (unsigned long long)parsed;
		return 0;
	}

	printf("Введите неотрицательное целое число (0..20): ");
	fflush(stdout);
	long long input = -1;
	if (scanf("%lld", &input) != 1) {
		return -1;
	}
	if (input < 0) {
		return -1;
	}
	*out_value = (unsigned long long)input;
	return 0;
}

static unsigned long long compute_factorial(unsigned int n) {
	unsigned long long result = 1ULL;
	for (unsigned int i = 2U; i <= n; ++i) {
		result *= i;
	}
	return result;
}

int main(int argc, char **argv) {
	unsigned long long n_value = 0ULL;
	if (read_non_negative_input(argc, argv, &n_value) != 0) {
		fprintf(stderr, "Ошибка: введите корректное неотрицательное целое число.\n");
		return 1;
	}

	if (n_value > 20ULL) {
		fprintf(stderr, "Ошибка: факториал %llu не помещается в 64-битное целое. Укажите число в диапазоне 0..20.\n", n_value);
		return 2;
	}

	unsigned long long result = compute_factorial((unsigned int)n_value);
	printf("%llu\n", result);
	return 0;
}


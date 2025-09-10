// Simple factorial program in C (iterative), supports input via argv or stdin
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

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
	char buffer[256];
	if (fgets(buffer, sizeof buffer, stdin) == NULL) {
		return -1;
	}
	char *ptr = buffer;
	while (*ptr != '\0' && isspace((unsigned char)*ptr)) {
		++ptr;
	}
	char *end_ptr = NULL;
	errno = 0;
	long long parsed = strtoll(ptr, &end_ptr, 10);
	if (errno != 0 || end_ptr == ptr) {
		return -1;
	}
	while (*end_ptr != '\0' && isspace((unsigned char)*end_ptr)) {
		++end_ptr;
	}
	if (*end_ptr != '\0') {
		return -1; // extra non-whitespace characters -> reject (e.g., fractional part)
	}
	if (parsed < 0) {
		return -1;
	}
	*out_value = (unsigned long long)parsed;
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


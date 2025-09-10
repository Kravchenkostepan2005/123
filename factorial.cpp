#include <iostream>
#include <cstdint>

// Вычисляет n! для 0 <= n <= 20 (вмещается в uint64_t)
static std::uint64_t factorial(unsigned int n) {
	std::uint64_t result = 1ULL;
	for (unsigned int i = 2; i <= n; ++i) {
		result *= static_cast<std::uint64_t>(i);
	}
	return result;
}

int main() {
	long long input;
	std::cout << "Введите целое число от 0 до 20: ";
	if (!(std::cin >> input)) {
		std::cerr << "Ошибка: не удалось прочитать число.\n";
		return 1;
	}

	if (input < 0) {
		std::cerr << "Ошибка: факториал определён только для неотрицательных чисел.\n";
		return 1;
	}
	if (input > 20) {
		std::cerr << "Ошибка: 21! и больше не помещается в 64-битный тип.\n";
		return 1;
	}

	unsigned int n = static_cast<unsigned int>(input);
	std::uint64_t value = factorial(n);
	std::cout << n << "! = " << value << '\n';
	return 0;
}


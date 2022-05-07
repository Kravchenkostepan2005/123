def add(x, y):
    return x + y
def subtract(x, y):
    return x - y
def multiply(x, y):
    return x * y
def divide(x, y):
    return x / y

print("Select operation: 1 - add, 2 - subtract, 3 - multiply, 4 - divide: ")


while True:
    choice = input("Enter choice(1/2/3/4): ")

    if choice not in ("1", "2", "3", "4"):
        print("invalid input")

    else:
        num1 = int(input("Enter first number: "))
        num2 = int(input("Enter second number: "))

    if choice == "1":
        print(num1, "+", num2, "=", add(num1, num2))
    elif choice == "2":
        print(num1, "-", num2, "=", subtract(num1, num2))
    elif choice == "3":
        print(num1, "*", num2, "=", multiply(num1, num2))
    elif choice == "4":
        print(num1, "/", num2, "=", divide(num1, num2))
        next_calculation = input("Let's do next calculation? (yes/no): ")
        if next_calculation == "no":
            break

def check(num1, num2):
    if num1.isdigit():
        print(num1)
    else:
        print("mistake")
    if num2.isdigit():
        print(num2)
    else:
        print("mistake")

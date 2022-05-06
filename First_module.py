from main import choice, add, subtract, multiply, divide

if choice not in ("1", "2", "3", "4"):
    print("invalid input")

else:
    num1 = float(input("Enter first number: "))
    num2 = float(input("Enter second number: "))

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
        print("Done")

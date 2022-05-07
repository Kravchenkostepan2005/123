from python1 import add, subtract, multiply, divide, check
num1 = float(input("Enter first number: "))
num2 = float(input("Enter second number: "))
print(add(num1, num2))
print(subtract(num1, num2))
print(multiply(num1, num2))
print(divide(num1, num2))




from python1 import add, subtract, multiply, divide, choice

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





class BankAccount:
    "basic class"
    def __init__(self, name, id, pin, balance=0):
        self.__name = name
        self.__id = id
        self.__pin = pin
        self.__balance = balance
        self.list_bank = {}
        self.name = name

    def __print_private_account1_details(self):
        return (self.__name, self.__id, self.__pin, self.__balance)

    def check_private_details(self):
        pin = input("Enter pin: ")
        if self.__pin == int(pin):
            return self.print_private_account1_details()
        if self.__pin != int(pin):
            return ('Wrong pin!')

    def change_pin_code(self):
        pin = input("Pin: ")
        if self.__pin == int(pin):
            self.__pin == pin

account_1 = BankAccount('User', 4698346, 6056, balance=10000)
print(account_1._BankAccount__print_private_account1_details())
print(account_1._BankAccount__name)
def add_bank(self, bank_name, money):
    self.list_bank[bank_name] = money
def getname(self):
    return self.name
def info(self):
    for key, val in self.list_bank.items():
        print(f'bank name - {key}, balance - {val}')
class ATM():
    "class inherited from BankAccount"
    bank_name = "MonoBank"
    def __init__(self, cash):
        self.cash = cash
        cash = 5000000

class Member:
    "basic class"
    def __init__(self, last_name = None, name = None, phone_number = None, from_line = None, city_village = None, email = None):
        if from_line is None:
            self.last_name = last_name
            self.name = name
            self.phone_number = phone_number
            self.city_village = city_village
            self.email = email
        else:
            self.last_name, self.name, self.phone_number, self.city_village, self.email = str(from_line).replace(" ", '').split("|")

    def input_characters(self):
        self.last_name = input("Enter last name: ").capitalize()
        self.name = input("Enter name: ").capitalize()
        self.phone_number = input("Enter phone number: ").capitalize()
        self.city_village = input("Enter city or village: ").capitalize()
        self.email = input("Enter email: ").capitalize()

    def __str__(self):
        return '{0:10} | (1:10} | {2}'.format(self.last_name, self.name, self.phone_number, self.city_village, self.email) + "\n"

class Contacts:
    "another class"
    def find_member(self, query):
        with open("data.txt") as file:
            for line in file:
                member = Member(from_line=line)
                if (member.last_name, member.name) == query:
                    return member
    def add_member(self):
        m = Member()
        m.input_characters()
        if c.find_member(query = (m.last_name, m.name)) is None:
            f = open("data.txt", "a")
            f.write('{0:10} | (1:10} | {2}'.format(self.last_name, self.name, self.phone_number) + "\n")
            print("\nКонтакт {lastName} {name}успешно добавлен\n".format(self.last_name == m.last_name, self.name == m.name))
            f.close()
        else:
            print("Такой контакт уже есть")

    def delete_member(self, query):
        objects = []
        f = open('data.txt', "r+")
        for line in f.readlines():
            member = Member(from_line = line)
            objects.append(member)
        for object in objects:
            if (member, last_name, member.name) != query:
                f.write(object.__str__())

    def show_all_contacts(self):
        with open("data.txt") as f1:
            for line in f1:
                member = Member(from_line=line)
                print(member)
c = Contacts()

def choice():
    selector = None
    try:
        selector = int(input("Enter 1 to find a contact\n" + \
                             "Enter 2 to add a contact\n" + \
                             "Enter 3 to delete a contact\n" + \
                             "Enter 4 to show all contacts\n" + \
                             "Enter here: "))
    except ValueError:
        print("\n\nWrong input!\n")
        print("Enter an integer!\n\n")
    return selector

c = Contacts()
while True:
    selector = choice()
    if selector == 1:
        query = ((input("Для поиска контакта введите его фамилию: ").capitalize(),
                  input("Для поиска контакта введите его имя: ").capitalize()))
        c.delete_member(query)
    elif selector == 4:
        c.show_all_contacts()

import telebot
from telebot import types

bot = telebot.TeleBot('5482343128:AAGKSxf6fVMwsQnMcJtAGjBFHIuTt5zqIjA')


@bot.message_handler(commands=['start', 'help'])
def send_welcome(message):
    bot.reply_to(message, "Hi! I'm the bot that can help you with your money. "
                          ""
                          " Now click on this button -> /button. "
                          "")


@bot.message_handler(commands=['button'])
def button(message):
    markup = types.InlineKeyboardMarkup(row_width=1)
    i1 = types.InlineKeyboardButton('Information', callback_data='one')
    i2 = types.InlineKeyboardButton("Career", callback_data='two')
    i3 = types.InlineKeyboardButton("Family", callback_data='three')
    i4 = types.InlineKeyboardButton("Surrounding ", callback_data='four')
    i5 = types.InlineKeyboardButton("Hobby", callback_data='five')
    i6 = types.InlineKeyboardButton("Relax and travelling", callback_data='six')
    i7 = types.InlineKeyboardButton("Education", callback_data='seven')
    i8 = types.InlineKeyboardButton("Health and sport", callback_data='eight')
    markup.add(i1, i2, i3, i4, i5, i6, i7, i8)

    bot.send_message(message.chat.id, 'Hi! Choose the button')


@bot.callback_query_handler(func=lambda call: True)
def callback(call):
    if call.message:
        if call.data == "one":
            bot.send_message(call.message.chat.id,
                             'My bot helps you with money you spend. You need to read instruction.Good luck:) ')
        elif call.data == 'two':
            msg = bot.send_message(call.message.chat.id, 'Print how much money you spend.')
            bot.register_next_step_handler(msg, save1)
        elif call.data == 'three':
            msg = bot.send_message(call.message.chat.id, 'Print how much money you spend.')
            bot.register_next_step_handler(msg, save2)
        elif call.data == 'four':
            msg = bot.send_message(call.message.chat.id, 'Print how much money you spend.')
            bot.register_next_step_handler(msg, save3)
        elif call.data == 'five':
            msg = bot.send_message(call.message.chat.id, 'Print how much money you spend.')
            bot.register_next_step_handler(msg, save4)
        elif call.data == 'six':
            msg = bot.send_message(call.message.chat.id, 'Print how much money you spend.')
            bot.register_next_step_handler(msg, save5)
        elif call.data == 'seven':
            msg = bot.send_message(call.message.chat.id, 'Print how much money you spend.')
            bot.register_next_step_handler(msg, save6)
        elif call.data == 'eight':
            msg = bot.send_message(call.message.chat.id, 'Print how much money you spend.')
            bot.register_next_step_handler(msg, save7)


def save1(message):
    with open('career.txt', 'w') as file:
        file.write(message.text)
    print("save")


def save2(message):
    with open('family.txt', 'w') as file:
        file.write(message.text)
    print("save")


def save3(message):
    with open('surrounding.txt', 'w') as file:
        file.write(message.text)
    print("save")


def save4(message):
    with open('hobby.txt', 'w') as file:
        file.write(message.text)
    print("save")


def save5(message):
    with open('travelling.txt', 'w') as file:
        file.write(message.text)
    print("save")


def save6(message):
    with open('education.txt', 'w') as file:
        file.write(message.text)
    print("save")


def save7(message):
    with open('health and sport.txt', 'w') as file:
        file.write(message.text)
    print("save")


bot.polling()

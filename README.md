## Дневник тренировок (Kivy, Python)

Мобильное приложение для планирования и ведения дневника тренировок с аналитикой.

Возможности:
- 5 дней тренировок в неделю + возможность добавить до 2 дополнительных дней вручную
- Планирование упражнений на неделю
- Фактический ввод по каждому подходу: повторы, вес, единицы (кг/фунты)
- Текущий вес тела для каждой тренировки
- Общий поднятый вес за тренировку
- Аналитика динамики по отдельным упражнениям и по группам мышц за периоды: неделя, месяц, 3 месяца, 6 месяцев, год
- Сохранение единиц веса по умолчанию в настройках

Технологии:
- Python 3.10+
- Kivy
- Peewee (SQLite)
- kivy_garden.graph для графиков

### Установка окружения (локально)
```bash
python3 -m venv .venv  # может потребоваться: sudo apt install python3.13-venv
source .venv/bin/activate
pip install -r requirements.txt --break-system-packages
python main.py
```

### Сборка для Android (Buildozer)
1. Установить buildozer и зависимости согласно документации Kivy
2. Сгенерировать spec-файл (шаблон `buildozer.spec` уже добавлен):
```bash
buildozer init
buildozer android debug
```

### Сборка для macOS (PyInstaller)
```bash
pip install pyinstaller
pyinstaller --name TrainingDiary --windowed --icon=assets/icon.icns main.py
```

> Примечание: для корректной работы БД на мобильных устройствах файл создается в `user_data_dir` приложения.

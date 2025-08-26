from models import MuscleGroup, Exercise, UserSettings


DEFAULT_MUSCLE_GROUPS = [
    "Грудь",
    "Спина",
    "Ноги",
    "Плечи",
    "Руки",
    "Кор",
]


DEFAULT_EXERCISES = [
    ("Жим лёжа", "Грудь"),
    ("Разводка гантелей", "Грудь"),
    ("Тяга штанги в наклоне", "Спина"),
    ("Подтягивания", "Спина"),
    ("Приседания", "Ноги"),
    ("Становая тяга", "Ноги"),
    ("Жим штанги стоя", "Плечи"),
    ("Подъём штанги на бицепс", "Руки"),
    ("Французский жим", "Руки"),
    ("Планка", "Кор"),
]


def seed_reference_data() -> None:
    for name in DEFAULT_MUSCLE_GROUPS:
        MuscleGroup.get_or_create(name=name)
    for ex_name, group_name in DEFAULT_EXERCISES:
        mg = MuscleGroup.get(MuscleGroup.name == group_name)
        Exercise.get_or_create(name=ex_name, muscle_group=mg)
    UserSettings.get_or_create(id=1)


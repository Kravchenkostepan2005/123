from peewee import Model, AutoField, CharField, IntegerField, ForeignKeyField, DateField, FloatField, BooleanField
from db import db_proxy


class BaseModel(Model):
    class Meta:
        database = db_proxy


class MuscleGroup(BaseModel):
    id = AutoField()
    name = CharField(unique=True)


class Exercise(BaseModel):
    id = AutoField()
    name = CharField(unique=True)
    muscle_group = ForeignKeyField(MuscleGroup, backref="exercises", on_delete="CASCADE")


class UserSettings(BaseModel):
    id = AutoField()
    default_weight_unit = CharField(default="kg")  # kg | lb


class TrainingPlanDay(BaseModel):
    id = AutoField()
    week_monday = DateField()  # понедельник недели, к которой относится план
    weekday_index = IntegerField()  # 0..6
    is_extra = BooleanField(default=False)  # доп. дни (6-й, 7-й)


class TrainingPlanExercise(BaseModel):
    id = AutoField()
    plan_day = ForeignKeyField(TrainingPlanDay, backref="plan_exercises", on_delete="CASCADE")
    exercise = ForeignKeyField(Exercise, backref="planned_in", on_delete="CASCADE")
    target_sets = IntegerField(default=3)
    target_reps = IntegerField(default=8)


class WorkoutSession(BaseModel):
    id = AutoField()
    date = DateField(index=True)
    body_weight_kg = FloatField(null=True)
    muscle_group = ForeignKeyField(MuscleGroup, backref="sessions", null=True)
    total_weight_kg = FloatField(default=0.0)


class WorkoutExercise(BaseModel):
    id = AutoField()
    session = ForeignKeyField(WorkoutSession, backref="exercises", on_delete="CASCADE")
    exercise = ForeignKeyField(Exercise, backref="workout_entries", on_delete="CASCADE")


class WorkoutSet(BaseModel):
    id = AutoField()
    workout_exercise = ForeignKeyField(WorkoutExercise, backref="sets", on_delete="CASCADE")
    reps = IntegerField(default=0)
    weight_kg = FloatField(default=0.0)
    unit = CharField(default="kg")  # сохранение выбранной единицы в момент ввода


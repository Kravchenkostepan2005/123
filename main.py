import os
from datetime import date
from pathlib import Path

from kivy.app import App
from kivy.lang import Builder
from kivy.properties import ListProperty, StringProperty
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.screenmanager import Screen
from kivy.uix.label import Label
from widgets import PlanDayRow, WorkoutExerciseCard, WorkoutSetRow

from db import initialize_database
from models import (
    MuscleGroup,
    Exercise,
    UserSettings,
    TrainingPlanDay,
    TrainingPlanExercise,
    WorkoutSession,
    WorkoutExercise,
    WorkoutSet,
)
from seed import seed_reference_data
from utils import week_range, get_monday, lb_to_kg, kg_to_lb, period_start


KV_PATH = str(Path(__file__).with_name("ui.kv"))


class RootScreen(BoxLayout):
    pass


class PlanScreen(Screen):
    week_label = StringProperty("")
    days_data = ListProperty([])

    def on_pre_enter(self, *args):
        self._ensure_week()
        self._refresh()

    def _ensure_week(self):
        today = date.today()
        monday = get_monday(today)
        self.current_monday = monday
        # создать 5 дней (пн-пт) плана если их нет
        existing = { (p.weekday_index, p.is_extra): p for p in TrainingPlanDay.select().where(TrainingPlanDay.week_monday == monday) }
        for i in range(5):
            if (i, False) not in existing:
                TrainingPlanDay.create(week_monday=monday, weekday_index=i, is_extra=False)

    def _refresh(self):
        monday = self.current_monday
        self.week_label = f"Неделя: {monday.isoformat()}"
        days = (TrainingPlanDay
                .select()
                .where(TrainingPlanDay.week_monday == monday)
                .order_by(TrainingPlanDay.weekday_index, TrainingPlanDay.is_extra))
        result = []
        weekdays = ["Пн","Вт","Ср","Чт","Пт","Сб","Вс"]
        for d in days:
            title = f"{weekdays[d.weekday_index]} (доп)" if d.is_extra else weekdays[d.weekday_index]
            result.append({
                'title': title,
                'plan_day_id': d.id,
            })
        self.days_data = result

    def change_week(self, delta_days: int):
        from datetime import timedelta
        self.current_monday = self.current_monday + timedelta(days=delta_days)
        self._ensure_week()
        self._refresh()

    def add_extra_day(self):
        # добавляем до 2 доп. дней (сб и вс)
        monday = self.current_monday
        days = list(TrainingPlanDay.select().where(TrainingPlanDay.week_monday == monday))
        extra_count = len([d for d in days if d.is_extra])
        if extra_count >= 2:
            return
        # назначаем субботу (5) затем воскресенье (6)
        next_index = 5 if not any(d.weekday_index == 5 for d in days) else 6
        TrainingPlanDay.create(week_monday=monday, weekday_index=next_index, is_extra=True)
        self._refresh()


class WorkoutScreen(Screen):
    date_label = StringProperty("")
    exercises_data = ListProperty([])
    total_label = StringProperty("0")

    def on_pre_enter(self, *args):
        self.current_date = date.today()
        self.date_label = self.current_date.isoformat()
        self._ensure_session()
        self._refresh()

    def _ensure_session(self):
        session, _ = WorkoutSession.get_or_create(date=self.current_date)
        self.session = session

    def save_body_weight(self, text: str):
        try:
            bw = float(text)
        except ValueError:
            return
        self.session.body_weight_kg = bw
        self.session.save()

    def add_from_plan(self):
        # находим план текущей недели и дня
        monday = get_monday(self.current_date)
        weekday_index = self.current_date.weekday()
        plan_day = (TrainingPlanDay
                    .select()
                    .where((TrainingPlanDay.week_monday == monday) & (TrainingPlanDay.weekday_index == weekday_index))
                    .first())
        if not plan_day:
            return
        for pe in plan_day.plan_exercises:
            we, _ = WorkoutExercise.get_or_create(session=self.session, exercise=pe.exercise)
            # если нет ни одного подхода, создадим таргетный
            if we.sets.count() == 0:
                WorkoutSet.create(workout_exercise=we, reps=pe.target_reps, weight_kg=0.0, unit=App.get_running_app().default_unit)
        self._refresh()
        App.get_running_app()._update_session_total(self.session)

    def _refresh(self):
        data = []
        for we in WorkoutExercise.select().where(WorkoutExercise.session == self.session):
            sets_data = []
            for s in we.sets:
                sets_data.append({
                    'set_id': s.id,
                    'reps': str(s.reps),
                    'weight': str(s.weight_kg if s.unit == 'kg' else kg_to_lb(s.weight_kg)),
                    'unit': s.unit,
                })
            data.append({
                'workout_exercise_id': we.id,
                'title': we.exercise.name,
                'sets': sets_data,
            })
        self.exercises_data = data
        self.total_label = f"Итого: {round(self.session.total_weight_kg, 1)} кг"


class AnalyticsScreen(Screen):
    selected_exercise_name = StringProperty("")
    selected_period = StringProperty("Месяц")

    def on_pre_enter(self, *args):
        self._ensure_exercise_values()

    def _ensure_exercise_values(self):
        app = App.get_running_app()
        app.exercise_names = [e.name for e in Exercise.select().order_by(Exercise.name)]

    def select_exercise(self, name: str):
        self.selected_exercise_name = name
        self._render_graph()

    def select_period(self, period: str):
        self.selected_period = period
        self._render_graph()

    def _render_graph(self):
        # Для MVP — просто считает общий вес по сессиям выбранного упражнения
        if not self.selected_exercise_name:
            return
        start_date = period_start(self.selected_period)
        ex = Exercise.select().where(Exercise.name == self.selected_exercise_name).first()
        if not ex:
            return
        # собираем суммы по дням
        from collections import defaultdict
        totals = defaultdict(float)
        query = (WorkoutSet
                 .select(WorkoutSet, WorkoutExercise, WorkoutSession)
                 .join(WorkoutExercise)
                 .switch(WorkoutExercise)
                 .join(WorkoutSession)
                 .where((WorkoutExercise.exercise == ex) & (WorkoutSession.date >= start_date)))
        for s in query:
            session_date = s.workout_exercise.session.date
            weight_kg = s.weight_kg
            totals[session_date] += weight_kg * s.reps
        # сохраняем в app для простого текстового отображения (заменим графиком позже)
        App.get_running_app().analytics_points = sorted(((d.isoformat(), round(w, 1)) for d, w in totals.items()))


class SettingsScreen(Screen):
    pass


class TrainingDiaryApp(App):
    default_unit = StringProperty("kg")
    exercise_names = ListProperty([])
    analytics_points = ListProperty([])

    def build(self):
        # БД путь
        user_dir = self.user_data_dir if hasattr(self, 'user_data_dir') else str(Path.home())
        db_path = os.path.join(user_dir, "training_diary.db")
        # Инициализация БД
        initialize_database(db_path, [
            MuscleGroup,
            Exercise,
            UserSettings,
            TrainingPlanDay,
            TrainingPlanExercise,
            WorkoutSession,
            WorkoutExercise,
            WorkoutSet,
        ])
        seed_reference_data()
        # Загрузка настроек
        settings = UserSettings.get_by_id(1)
        self.default_unit = settings.default_weight_unit

        Builder.load_file(KV_PATH)
        return RootScreen()

    def switch_screen(self, name: str):
        root = self.root
        sm = root.ids.get('sm')
        if sm:
            sm.current = name

    def update_default_unit(self, unit: str):
        self.default_unit = unit
        s = UserSettings.get_by_id(1)
        s.default_weight_unit = unit
        s.save()

    # План
    def add_exercise_to_plan(self, plan_day_id: int):
        # MVP: добавим первое упражнение из списка
        ex = Exercise.select().order_by(Exercise.id).first()
        if not ex:
            return
        pd = TrainingPlanDay.get_by_id(plan_day_id)
        TrainingPlanExercise.create(plan_day=pd, exercise=ex)

    def open_day_workout(self, plan_day_id: int):
        self.switch_screen("workout")

    # Тренировка
    def add_set_to_exercise(self, workout_exercise_id: int):
        we = WorkoutExercise.get_by_id(workout_exercise_id)
        WorkoutSet.create(workout_exercise=we, reps=8, weight_kg=0.0, unit=self.default_unit)
        self._update_session_total(we.session)
        self._refresh_workout()

    def update_set(self, set_id: int, reps_text: str | None, weight_text: str | None, unit: str | None):
        s = WorkoutSet.get_by_id(set_id)
        if reps_text is not None and reps_text != "":
            try:
                s.reps = int(reps_text)
            except ValueError:
                pass
        if weight_text is not None and weight_text != "":
            try:
                if (unit or s.unit) == 'kg':
                    s.weight_kg = float(weight_text)
                else:
                    s.weight_kg = lb_to_kg(float(weight_text))
            except ValueError:
                pass
        if unit is not None:
            s.unit = unit
        s.save()
        self._update_session_total(s.workout_exercise.session)
        self._refresh_workout()

    def delete_set(self, set_id: int):
        s = WorkoutSet.get_by_id(set_id)
        session = s.workout_exercise.session
        s.delete_instance()
        self._update_session_total(session)
        self._refresh_workout()

    def _update_session_total(self, session: WorkoutSession):
        total = 0.0
        for we in session.exercises:
            for s in we.sets:
                total += s.weight_kg * s.reps
        session.total_weight_kg = total
        session.save()

    def _refresh_workout(self):
        # Обновить экран тренировки если он активен
        root = self.root
        sm = root.ids.get('sm')
        if sm and sm.current == 'workout':
            screen = sm.get_screen('workout')
            screen._refresh()


if __name__ == '__main__':
    TrainingDiaryApp().run()

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





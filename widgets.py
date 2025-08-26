from kivy.uix.boxlayout import BoxLayout
from kivy.properties import StringProperty, NumericProperty, ListProperty


class PlanDayRow(BoxLayout):
    title = StringProperty("")
    plan_day_id = NumericProperty(0)


class WorkoutExerciseCard(BoxLayout):
    title = StringProperty("")
    workout_exercise_id = NumericProperty(0)
    sets = ListProperty([])


class WorkoutSetRow(BoxLayout):
    set_id = NumericProperty(0)
    reps = StringProperty("")
    weight = StringProperty("")
    unit = StringProperty("kg")


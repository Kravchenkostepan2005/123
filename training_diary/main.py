from kivy.app import App
from kivy.uix.screenmanager import ScreenManager, NoTransition
from kivy.core.window import Window

from screens.planner import PlannerScreen
from screens.workout import WorkoutScreen
from screens.analytics import AnalyticsScreen


class RootScreenManager(ScreenManager):
	def __init__(self, **kwargs):
		super().__init__(**kwargs)
		self.transition = NoTransition()
		self.add_widget(PlannerScreen(name="planner"))
		self.add_widget(WorkoutScreen(name="workout"))
		self.add_widget(AnalyticsScreen(name="analytics"))


class TrainingDiaryApp(App):
	def build(self):
		Window.size = (420, 840)
		return RootScreenManager()


if __name__ == "__main__":
	TrainingDiaryApp().run()
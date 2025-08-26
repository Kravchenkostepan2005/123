from typing import Optional
from datetime import date, timedelta

from kivy.uix.screenmanager import Screen
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.spinner import Spinner
from kivy.uix.button import Button
from kivy.uix.label import Label

from training_diary.services.repository import Repository
from training_diary.services.analytics import exercise_volume_timeseries, workout_total_by_primary_group

# Matplotlib canvas
try:
	from kivy_garden.matplotlib.backend_kivyagg import FigureCanvasKivyAgg
except Exception:  # fallback if garden not available
	FigureCanvasKivyAgg = None
import matplotlib.pyplot as plt


PERIODS = {
	"Неделя": 7,
	"Месяц": 30,
	"3 месяца": 90,
	"6 месяцев": 180,
	"Год": 365,
}


class AnalyticsScreen(Screen):
	def __init__(self, **kwargs):
		super().__init__(**kwargs)
		self.repo = Repository()
		self.root_box = BoxLayout(orientation='vertical', spacing=8, padding=8)
		self.add_widget(self.root_box)
		self._build_ui()

	def _build_ui(self):
		controls = BoxLayout(size_hint_y=None, height=48, spacing=8)
		self.period_spinner = Spinner(text='Неделя', values=list(PERIODS.keys()), size_hint_x=None, width=140)
		self.exercise_spinner = Spinner(text='Упражнение', values=[e.name for e in self.repo.list_exercises()], size_hint_x=None, width=240)
		apply_btn = Button(text='Показать')
		apply_btn.bind(on_release=lambda *_: self._render())
		controls.add_widget(self.period_spinner)
		controls.add_widget(self.exercise_spinner)
		controls.add_widget(apply_btn)
		self.root_box.add_widget(controls)

		self.chart_box = BoxLayout()
		self.root_box.add_widget(self.chart_box)
		self.summary_lbl = Label(text='')
		self.root_box.add_widget(self.summary_lbl)

	def _render(self):
		period_days = PERIODS.get(self.period_spinner.text, 7)
		end = date.today()
		start = end - timedelta(days=period_days)
		ex = next((e for e in self.repo.list_exercises() if e.name == self.exercise_spinner.text), None)
		self.chart_box.clear_widgets()
		if not ex:
			self.summary_lbl.text = 'Выберите упражнение'
			return
		series = exercise_volume_timeseries(ex.id, start.strftime('%Y-%m-%d'), end.strftime('%Y-%m-%d'))
		if FigureCanvasKivyAgg and series:
			fig, ax = plt.subplots(figsize=(6, 3), dpi=100)
			x = [d for d, _ in series]
			y = [v for _, v in series]
			ax.plot(x, y, marker='o')
			ax.set_title(f'Тоннаж: {ex.name}')
			ax.set_xlabel('Дата')
			ax.set_ylabel('кг')
			ax.grid(True, alpha=0.3)
			self.chart_box.add_widget(FigureCanvasKivyAgg(fig))
		# summary by categories
		cats = workout_total_by_primary_group(start.strftime('%Y-%m-%d'), end.strftime('%Y-%m-%d'))
		if cats:
			s = ", ".join([f"{name}: {val:.0f} кг" for name, val in cats])
			self.summary_lbl.text = f"По категориям: {s}"
		else:
			self.summary_lbl.text = 'По категориям: нет данных'
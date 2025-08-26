from typing import Dict, List, Tuple
from datetime import date

from functools import partial

from kivy.uix.screenmanager import Screen
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.label import Label

from training_diary.services.repository import Repository
from training_diary.utils.dates import today_monday, week_range_str, add_days, shift_weeks


DAY_NAMES = [
	"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"
]


class PlannerScreen(Screen):
	def __init__(self, **kwargs):
		super().__init__(**kwargs)
		self.repo = Repository()
		self.current_week_monday = today_monday()
		self.week_id = None
		self._ensure_defaults()
		self._build_ui()
		self._load_or_create_week()

	def _ensure_defaults(self):
		self.repo.ensure_default_groups()

	def _build_ui(self):
		root = BoxLayout(orientation='vertical', padding=8, spacing=8)
		# Week header
		hdr = BoxLayout(size_hint_y=None, height=48, spacing=8)
		prev_btn = Button(text='⟨ Неделя', size_hint_x=None, width=120)
		next_btn = Button(text='Неделя ⟩', size_hint_x=None, width=120)
		self.week_lbl = Label(text='Неделя', halign='center', valign='middle')
		self.week_lbl.bind(size=lambda *_: setattr(self.week_lbl, 'text_size', self.week_lbl.size))
		prev_btn.bind(on_release=lambda *_: self._shift_week(-1))
		next_btn.bind(on_release=lambda *_: self._shift_week(1))
		hdr.add_widget(prev_btn)
		hdr.add_widget(self.week_lbl)
		hdr.add_widget(next_btn)
		root.add_widget(hdr)

		# Days grid
		self.days_grid = GridLayout(cols=2, spacing=8, size_hint_y=1)
		root.add_widget(self.days_grid)
		self.add_widget(root)

	def _shift_week(self, delta_weeks: int):
		self.current_week_monday = shift_weeks(self.current_week_monday, delta_weeks)
		self._load_or_create_week()

	def _load_or_create_week(self):
		# Try find week
		weeks = self.repo.list_weeks()
		wk = next((w for w in weeks if w[1] == self.current_week_monday), None)
		if wk is None:
			self.week_id = self.repo.create_week(self.current_week_monday)
			# create 5 default workouts (Mon-Fri)
			for i in range(5):
				self.repo.create_workout(self.week_id, i)
		else:
			self.week_id = wk[0]
		self._refresh_view()

	def _refresh_view(self):
		self.week_lbl.text = f"{week_range_str(self.current_week_monday)}"
		self.days_grid.clear_widgets()
		# Load workouts for week
		workouts = self.repo.workouts_for_week(self.week_id)
		by_day = {w[1]: w for w in workouts}  # plan_day_index -> row
		active_count = len(workouts)
		for day_idx in range(7):
			box = BoxLayout(orientation='vertical', padding=6, spacing=6)
			label = Label(text=f"{DAY_NAMES[day_idx]}\n{add_days(self.current_week_monday, day_idx)}", size_hint_y=None, height=48)
			box.add_widget(label)
			row = by_day.get(day_idx)
			if row:
				open_btn = Button(text='Открыть тренировку')
				open_btn.bind(on_release=partial(self._open_workout, row[0]))
				box.add_widget(open_btn)
			else:
				can_add = active_count < 7
				btn = Button(text='Добавить день' if can_add else 'Лимит 7 дней', disabled=not can_add)
				btn.bind(on_release=partial(self._add_day, day_idx))
				box.add_widget(btn)
			self.days_grid.add_widget(box)

	def _add_day(self, day_idx: int, *args):
		self.repo.create_workout(self.week_id, day_idx)
		self._refresh_view()

	def _open_workout(self, workout_id: int, *args):
		self.manager.current = 'workout'
		self.manager.get_screen('workout').load_workout(workout_id)
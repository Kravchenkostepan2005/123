from typing import List, Optional

from functools import partial

from kivy.uix.screenmanager import Screen
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.scrollview import ScrollView
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.label import Label
from kivy.uix.textinput import TextInput
from kivy.uix.spinner import Spinner
from kivy.uix.popup import Popup

from training_diary.services.repository import Repository
from training_diary.utils.units import lb_to_kg, kg_to_lb, format_weight


class WorkoutScreen(Screen):
	def __init__(self, **kwargs):
		super().__init__(**kwargs)
		self.repo = Repository()
		self.workout_id: Optional[int] = None
		self.weight_unit = 'kg'
		self.root_box = BoxLayout(orientation='vertical')
		self.add_widget(self.root_box)

	def load_workout(self, workout_id: int):
		self.workout_id = workout_id
		self._build_ui()

	def _build_ui(self):
		self.root_box.clear_widgets()
		# header
		hdr = BoxLayout(size_hint_y=None, height=48, spacing=8, padding=8)
		back_btn = Button(text='← Назад')
		back_btn.bind(on_release=lambda *_: self._go_back())
		mgroups = self.repo.list_muscle_groups()
		mg_names = [m.name for m in mgroups]
		self.mg_spinner = Spinner(text='Категория', values=mg_names, size_hint_x=None, width=180)
		self.mg_spinner.bind(text=lambda *_: self._on_group_changed())
		hdr.add_widget(back_btn)
		hdr.add_widget(Label(text='Тренировка', halign='left'))
		hdr.add_widget(self.mg_spinner)
		self.root_box.add_widget(hdr)

		# body weight control
		bw_box = BoxLayout(size_hint_y=None, height=48, padding=8, spacing=8)
		bw_box.add_widget(Label(text='Вес тела'))
		self.bw_input = TextInput(text='0', input_filter='float', multiline=False)
		unit_spinner = Spinner(text=self.weight_unit, values=['kg', 'lb'], size_hint_x=None, width=80)
		unit_spinner.bind(text=lambda _w, v: self._change_body_weight_unit(v))
		save_bw_btn = Button(text='Сохранить')
		save_bw_btn.bind(on_release=lambda *_: self._save_body_weight())
		bw_box.add_widget(self.bw_input)
		bw_box.add_widget(unit_spinner)
		bw_box.add_widget(save_bw_btn)
		self.root_box.add_widget(bw_box)

		# scroll area for exercises and sets
		sv = ScrollView()
		content = GridLayout(cols=1, size_hint_y=None, spacing=8, padding=8)
		content.bind(minimum_height=content.setter('height'))
		self.content = content
		sv.add_widget(content)
		self.root_box.add_widget(sv)

		# add exercise button
		add_ex_btn = Button(text='Добавить упражнение', size_hint_y=None, height=48)
		add_ex_btn.bind(on_release=lambda *_: self._open_add_exercise_popup())
		self.root_box.add_widget(add_ex_btn)

		# footer with total
		self.total_lbl = Label(text='Итого: 0 кг')
		self.root_box.add_widget(self.total_lbl)

		self._reload_workout_details()

	def _go_back(self, *args):
		self.manager.current = 'planner'

	def _change_body_weight_unit(self, unit: str):
		try:
			val = float(self.bw_input.text or '0')
		except Exception:
			val = 0
		if unit == 'kg' and self.weight_unit == 'lb':
			val = lb_to_kg(val)
		elif unit == 'lb' and self.weight_unit == 'kg':
			val = kg_to_lb(val)
		self.weight_unit = unit
		self.bw_input.text = f"{val:.1f}"

	def _save_body_weight(self, *args):
		try:
			val = float(self.bw_input.text or '0')
		except Exception:
			val = 0
		kg = val if self.weight_unit == 'kg' else lb_to_kg(val)
		self.repo.update_workout_body_weight(self.workout_id, kg)

	def _on_group_changed(self, *args):
		name = self.mg_spinner.text
		mg = next((g for g in self.repo.list_muscle_groups() if g.name == name), None)
		if mg:
			self.repo.update_workout_primary_group(self.workout_id, mg.id)

	def _reload_workout_details(self):
		self.content.clear_widgets()
		details = self.repo.get_workout_details(self.workout_id)
		# init header fields
		if details.get('primary_group_name'):
			self.mg_spinner.text = details['primary_group_name']
		else:
			self.mg_spinner.text = 'Категория'
		bw = details.get('body_weight_kg') or 0
		self.weight_unit = 'kg'
		self.bw_input.text = f"{bw:.1f}"
		# Exercises
		for item in details['items']:
			box = BoxLayout(orientation='vertical', size_hint_y=None, height=48 + 40 * max(1, len(item['sets'])) + 40, padding=4, spacing=4)
			head = BoxLayout(size_hint_y=None, height=40)
			head.add_widget(Label(text=item['exercise_name']))
			add_set_btn = Button(text='Добавить подход', size_hint_x=None, width=160)
			add_set_btn.bind(on_release=partial(self._add_set, item['we_id'], len(item['sets'])))
			head.add_widget(add_set_btn)
			box.add_widget(head)
			# sets
			for s in item['sets']:
				row = BoxLayout(size_hint_y=None, height=36, spacing=4)
				row.add_widget(Label(text=f"Подход {s['set_index']+1}"))
				reps_planned = TextInput(text=str(s['reps_planned'] or ''), input_filter='int', multiline=False)
				reps_actual = TextInput(text=str(s['reps_actual'] or ''), input_filter='int', multiline=False)
				weight_val = TextInput(text=str(s['weight_display']), input_filter='float', multiline=False)
				unit_spinner = Spinner(text=s['weight_unit'], values=['kg', 'lb'], size_hint_x=None, width=80)
				save_btn = Button(text='OK', size_hint_x=None, width=60)
				# bind save
				save_btn.bind(on_release=partial(self._save_set, s['id'], s['workout_exercise_id'], s['set_index'], reps_planned, reps_actual, weight_val, unit_spinner))
				row.add_widget(Label(text='План'))
				row.add_widget(reps_planned)
				row.add_widget(Label(text='Факт'))
				row.add_widget(reps_actual)
				row.add_widget(Label(text='Вес'))
				row.add_widget(weight_val)
				row.add_widget(unit_spinner)
				row.add_widget(save_btn)
				box.add_widget(row)
			self.content.add_widget(box)
		# update total
		total_kg = self.repo.total_lifted_weight_kg_for_workout(self.workout_id)
		self.total_lbl.text = f"Итого: {total_kg:.1f} кг / {kg_to_lb(total_kg):.1f} lb"

	def _add_set(self, workout_exercise_id: int, current_count: int, *args):
		self.repo.add_set(workout_exercise_id, current_count, None, None, None, 'kg')
		self._reload_workout_details()

	def _open_add_exercise_popup(self, *args):
		content = BoxLayout(orientation='vertical', spacing=4, padding=4)
		mg_spinner = Spinner(text='Группа', values=[g.name for g in self.repo.list_muscle_groups()])
		ex_spinner = Spinner(text='Упражнение', values=[])
		def on_mg_change(_w, text):
			mg = next((g for g in self.repo.list_muscle_groups() if g.name == text), None)
			if mg:
				ex_spinner.values = [e.name for e in self.repo.list_exercises(mg.id)]
		mg_spinner.bind(text=on_mg_change)
		add_btn = Button(text='Добавить')
		popup = Popup(title='Добавить упражнение', content=content, size_hint=(0.9, 0.6))
		def on_add(_w):
			ex = next((e for e in self.repo.list_exercises() if e.name == ex_spinner.text), None)
			if ex:
				order_index = self.repo.next_order_index(self.workout_id)
				we_id = self.repo.add_workout_exercise(self.workout_id, ex.id, order_index)
				self.repo.add_set(we_id, 0, None, None, None, 'kg')
				popup.dismiss()
				self._reload_workout_details()
		add_btn.bind(on_release=on_add)
		content.add_widget(mg_spinner)
		content.add_widget(ex_spinner)
		content.add_widget(add_btn)
		popup.open()

	def _save_set(self, set_id: int, workout_exercise_id: int, set_index: int, reps_planned_input: TextInput, reps_actual_input: TextInput, weight_value_input: TextInput, unit_spinner: Spinner, *args):
		def to_int(s):
			try:
				return int(s)
			except Exception:
				return None
		def to_float(s):
			try:
				return float(s)
			except Exception:
				return None
		reps_p = to_int(reps_planned_input.text)
		reps_a = to_int(reps_actual_input.text)
		w_val = to_float(weight_value_input.text) or 0.0
		unit = unit_spinner.text
		w_kg = w_val if unit == 'kg' else lb_to_kg(w_val)
		self.repo.update_set(set_id, reps_p, reps_a, w_kg, unit)
		self._reload_workout_details()
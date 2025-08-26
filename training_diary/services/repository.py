from dataclasses import dataclass
from typing import List, Optional, Tuple, Dict, Any
from datetime import date

from .db import get_db
from ..utils.units import kg_to_lb


@dataclass
class MuscleGroup:
	id: int
	name: str


@dataclass
class Exercise:
	id: int
	name: str
	muscle_group_id: int


class Repository:
	def __init__(self):
		self.db = get_db()

	# Settings
	def set_setting(self, key: str, value: str) -> None:
		self.db.execute("INSERT INTO settings(key, value) VALUES(?, ?) ON CONFLICT(key) DO UPDATE SET value=excluded.value", (key, value))

	def get_setting(self, key: str, default: Optional[str] = None) -> Optional[str]:
		row = self.db.execute("SELECT value FROM settings WHERE key=?", (key,)).fetchone()
		return row[0] if row else default

	# Muscle groups & exercises
	def ensure_default_groups(self) -> None:
		defaults = [
			"Грудь", "Спина", "Ноги", "Плечи", "Руки", "Кор"
		]
		for name in defaults:
			self.db.execute("INSERT OR IGNORE INTO muscle_groups(name) VALUES(?)", (name,))

	def add_exercise(self, name: str, muscle_group_id: int) -> int:
		cur = self.db.execute("INSERT INTO exercises(name, muscle_group_id) VALUES(?, ?)", (name, muscle_group_id))
		return cur.lastrowid

	def list_muscle_groups(self) -> List[MuscleGroup]:
		rows = self.db.execute("SELECT id, name FROM muscle_groups ORDER BY name").fetchall()
		return [MuscleGroup(id=row[0], name=row[1]) for row in rows]

	def list_exercises(self, group_id: Optional[int] = None) -> List[Exercise]:
		if group_id is None:
			rows = self.db.execute("SELECT id, name, muscle_group_id FROM exercises ORDER BY name").fetchall()
		else:
			rows = self.db.execute("SELECT id, name, muscle_group_id FROM exercises WHERE muscle_group_id=? ORDER BY name", (group_id,)).fetchall()
		return [Exercise(id=r[0], name=r[1], muscle_group_id=r[2]) for r in rows]

	# Weeks & workouts
	def create_week(self, start_date: str) -> int:
		cur = self.db.execute("INSERT INTO weeks(start_date) VALUES(?)", (start_date,))
		return cur.lastrowid

	def list_weeks(self) -> List[Tuple[int, str]]:
		rows = self.db.execute("SELECT id, start_date FROM weeks ORDER BY start_date DESC").fetchall()
		return [(r[0], r[1]) for r in rows]

	def workouts_for_week(self, week_id: int) -> List[Tuple[int, int]]:
		rows = self.db.execute("SELECT id, plan_day_index FROM workouts WHERE week_id=? ORDER BY plan_day_index", (week_id,)).fetchall()
		return [(r[0], r[1]) for r in rows]

	def create_workout(self, week_id: int, plan_day_index: int, body_weight_kg: float = 0.0, note: str = "") -> int:
		cur = self.db.execute(
			"INSERT INTO workouts(week_id, plan_day_index, body_weight_kg, note) VALUES(?, ?, ?, ?)",
			(week_id, plan_day_index, body_weight_kg, note),
		)
		return cur.lastrowid

	def update_workout_body_weight(self, workout_id: int, weight_kg: float) -> None:
		self.db.execute("UPDATE workouts SET body_weight_kg=? WHERE id=?", (weight_kg, workout_id))
		self.touch_workout_actual_date(workout_id)

	def update_workout_primary_group(self, workout_id: int, group_id: int) -> None:
		self.db.execute("UPDATE workouts SET primary_muscle_group_id=? WHERE id=?", (group_id, workout_id))
		self.touch_workout_actual_date(workout_id)

	def touch_workout_actual_date(self, workout_id: int) -> None:
		# Устанавливаем текущую дату, если пусто
		row = self.db.execute("SELECT actual_date FROM workouts WHERE id=?", (workout_id,)).fetchone()
		if row and (row[0] is None or row[0] == ""):
			today = date.today().strftime('%Y-%m-%d')
			self.db.execute("UPDATE workouts SET actual_date=? WHERE id=?", (today, workout_id))

	def add_workout_exercise(self, workout_id: int, exercise_id: int, order_index: int) -> int:
		cur = self.db.execute(
			"INSERT INTO workout_exercises(workout_id, exercise_id, order_index) VALUES(?, ?, ?)",
			(workout_id, exercise_id, order_index),
		)
		return cur.lastrowid

	def next_order_index(self, workout_id: int) -> int:
		row = self.db.execute("SELECT COALESCE(MAX(order_index), -1) + 1 FROM workout_exercises WHERE workout_id=?", (workout_id,)).fetchone()
		return int(row[0]) if row else 0

	def add_set(self, workout_exercise_id: int, set_index: int, reps_planned: Optional[int], reps_actual: Optional[int], weight_kg: Optional[float], weight_unit: str = "kg") -> int:
		cur = self.db.execute(
			"INSERT INTO sets(workout_exercise_id, set_index, reps_planned, reps_actual, weight_kg, weight_unit) VALUES(?, ?, ?, ?, ?, ?)",
			(workout_exercise_id, set_index, reps_planned, reps_actual, weight_kg, weight_unit),
		)
		return cur.lastrowid

	def update_set(self, set_id: int, reps_planned: Optional[int], reps_actual: Optional[int], weight_kg: Optional[float], weight_unit: str) -> None:
		self.db.execute(
			"UPDATE sets SET reps_planned=?, reps_actual=?, weight_kg=?, weight_unit=? WHERE id=?",
			(reps_planned, reps_actual, weight_kg, weight_unit, set_id),
		)
		# также проставим дату тренировки
		row = self.db.execute("SELECT we.workout_id FROM sets s JOIN workout_exercises we ON we.id = s.workout_exercise_id WHERE s.id=?", (set_id,)).fetchone()
		if row:
			self.touch_workout_actual_date(int(row[0]))

	def get_workout_details(self, workout_id: int) -> Dict[str, Any]:
		w = self.db.execute(
			"SELECT w.id, w.body_weight_kg, w.primary_muscle_group_id, mg.name as mg_name FROM workouts w LEFT JOIN muscle_groups mg ON mg.id = w.primary_muscle_group_id WHERE w.id=?",
			(workout_id,),
		).fetchone()
		res: Dict[str, Any] = {
			"id": workout_id,
			"body_weight_kg": float(w[1]) if w else 0.0,
			"primary_group_id": int(w[2]) if w and w[2] is not None else None,
			"primary_group_name": w[3] if w else None,
			"items": [],
		}
		rows = self.db.execute(
			"SELECT we.id as we_id, we.exercise_id, e.name, we.order_index FROM workout_exercises we JOIN exercises e ON e.id = we.exercise_id WHERE we.workout_id=? ORDER BY we.order_index",
			(workout_id,),
		).fetchall()
		for r in rows:
			we_id = int(r[0])
			item = {
				"we_id": we_id,
				"exercise_id": int(r[1]),
				"exercise_name": r[2],
				"order_index": int(r[3]),
				"sets": [],
			}
			sets = self.db.execute(
				"SELECT id, workout_exercise_id, set_index, reps_planned, reps_actual, weight_kg, weight_unit FROM sets WHERE workout_exercise_id=? ORDER BY set_index",
				(we_id,),
			).fetchall()
			for s in sets:
				wkg = s[5] if s[5] is not None else 0.0
				unit = s[6]
				wdisp = (wkg if unit == 'kg' else kg_to_lb(wkg)) if wkg is not None else 0.0
				item["sets"].append({
					"id": int(s[0]),
					"workout_exercise_id": int(s[1]),
					"set_index": int(s[2]),
					"reps_planned": int(s[3]) if s[3] is not None else None,
					"reps_actual": int(s[4]) if s[4] is not None else None,
					"weight_kg": float(wkg) if wkg is not None else None,
					"weight_unit": unit,
					"weight_display": round(float(wdisp), 2),
				})
			res["items"].append(item)
		return res

	def total_lifted_weight_kg_for_workout(self, workout_id: int) -> float:
		row = self.db.execute(
			"SELECT COALESCE(SUM(COALESCE(reps_actual, reps_planned, 0) * COALESCE(weight_kg, 0)), 0) FROM sets s JOIN workout_exercises we ON we.id = s.workout_exercise_id WHERE we.workout_id=?",
			(workout_id,),
		).fetchone()
		return float(row[0]) if row else 0.0
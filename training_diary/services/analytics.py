from datetime import datetime, timedelta
from typing import List, Tuple, Optional, Dict

from training_diary.services.db import get_db


def _to_date(s: str):
	return datetime.strptime(s, "%Y-%m-%d").date()


def exercise_volume_timeseries(exercise_id: int, start_date: str, end_date: str) -> List[Tuple[str, float]]:
	"""
	Возвращает [(дата, тоннаж_кг)] по дням за период.
	"""
	db = get_db()
	rows = db.execute(
		"""
		SELECT w.actual_date, COALESCE(SUM(COALESCE(s.reps_actual, s.reps_planned, 0) * COALESCE(s.weight_kg, 0)), 0) AS volume
		FROM workouts w
		JOIN workout_exercises we ON we.workout_id = w.id
		JOIN sets s ON s.workout_exercise_id = we.id
		WHERE we.exercise_id = ? AND w.actual_date BETWEEN ? AND ?
		GROUP BY w.actual_date
		ORDER BY w.actual_date
		""",
		(exercise_id, start_date, end_date),
	).fetchall()
	return [(r[0], float(r[1])) for r in rows]


def workout_total_by_primary_group(start_date: str, end_date: str) -> List[Tuple[str, float]]:
	"""
	Суммарный тоннаж тренировок, сгруппированный по primary_muscle_group_id за период.
	Возвращает [(название_группы, тоннаж_кг)].
	"""
	db = get_db()
	rows = db.execute(
		"""
		SELECT mg.name, COALESCE(SUM((
			SELECT COALESCE(SUM(COALESCE(s.reps_actual, s.reps_planned, 0) * COALESCE(s.weight_kg, 0)), 0)
			FROM workout_exercises we2 JOIN sets s ON s.workout_exercise_id = we2.id
			WHERE we2.workout_id = w.id
		)), 0) AS total_volume
		FROM workouts w
		LEFT JOIN muscle_groups mg ON mg.id = w.primary_muscle_group_id
		WHERE w.actual_date BETWEEN ? AND ? AND w.primary_muscle_group_id IS NOT NULL
		GROUP BY mg.name
		ORDER BY total_volume DESC
		""",
		(start_date, end_date),
	).fetchall()
	return [(r[0], float(r[1])) for r in rows]
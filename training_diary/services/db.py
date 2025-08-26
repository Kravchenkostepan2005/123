import sqlite3
from pathlib import Path
from typing import Iterable, Optional

DB_PATH = Path(__file__).resolve().parent.parent / "training_diary.db"


SCHEMA_SQL = """
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS settings (
	key TEXT PRIMARY KEY,
	value TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS muscle_groups (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS exercises (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL,
	muscle_group_id INTEGER NOT NULL,
	UNIQUE(name),
	FOREIGN KEY (muscle_group_id) REFERENCES muscle_groups(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS weeks (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	start_date TEXT NOT NULL -- ISO yyyy-mm-dd (пн)
);

CREATE TABLE IF NOT EXISTS workouts (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	week_id INTEGER NOT NULL,
	plan_day_index INTEGER NOT NULL, -- 0..6
	actual_date TEXT, -- ISO дата фактически
	body_weight_kg REAL NOT NULL DEFAULT 0,
	note TEXT,
	-- primary_muscle_group_id будет добавлен миграцией
	FOREIGN KEY (week_id) REFERENCES weeks(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS workout_exercises (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	workout_id INTEGER NOT NULL,
	exercise_id INTEGER NOT NULL,
	order_index INTEGER NOT NULL,
	FOREIGN KEY (workout_id) REFERENCES workouts(id) ON DELETE CASCADE,
	FOREIGN KEY (exercise_id) REFERENCES exercises(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS sets (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	workout_exercise_id INTEGER NOT NULL,
	set_index INTEGER NOT NULL,
	reps_planned INTEGER,
	reps_actual INTEGER,
	weight_kg REAL,
	weight_unit TEXT NOT NULL DEFAULT 'kg', -- 'kg'|'lb' исходный ввод
	FOREIGN KEY (workout_exercise_id) REFERENCES workout_exercises(id) ON DELETE CASCADE
);
"""


class Database:
	def __init__(self, path: Optional[Path] = None):
		self.path = Path(path) if path else DB_PATH
		self.path.parent.mkdir(parents=True, exist_ok=True)
		self._ensure_schema()

	def connect(self) -> sqlite3.Connection:
		conn = sqlite3.connect(self.path)
		conn.row_factory = sqlite3.Row
		return conn

	def _ensure_schema(self) -> None:
		with self.connect() as conn:
			conn.executescript(SCHEMA_SQL)
			# миграция: добавить workouts.primary_muscle_group_id если нет
			cols = [r[1] for r in conn.execute("PRAGMA table_info(workouts)").fetchall()]
			if "primary_muscle_group_id" not in cols:
				conn.execute("ALTER TABLE workouts ADD COLUMN primary_muscle_group_id INTEGER NULL")
				conn.commit()

	def execute(self, sql: str, params: Iterable = ()):  # -> sqlite3.Cursor
		with self.connect() as conn:
			cur = conn.execute(sql, params)
			conn.commit()
			return cur

	db = None  # singleton helper


def get_db() -> Database:
	global db
	if db is None:
		db = Database()
	return db
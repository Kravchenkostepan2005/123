from dataclasses import dataclass, field
from typing import List, Optional


@dataclass
class WorkoutSet:
	id: int
	set_index: int
	reps_planned: Optional[int]
	reps_actual: Optional[int]
	weight_kg: Optional[float]
	weight_unit: str = "kg"


@dataclass
class WorkoutExerciseItem:
	id: int
	exercise_id: int
	exercise_name: str
	order_index: int
	sets: List[WorkoutSet] = field(default_factory=list)


@dataclass
class WorkoutModel:
	id: int
	week_id: int
	plan_day_index: int
	actual_date: Optional[str]
	body_weight_kg: float
	note: str = ""
	primary_muscle_group_id: Optional[int] = None
	items: List[WorkoutExerciseItem] = field(default_factory=list)
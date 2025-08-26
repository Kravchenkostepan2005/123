KG_IN_LB = 0.45359237
LB_IN_KG = 2.20462262185


def kg_to_lb(kg: float) -> float:
	return round(kg / KG_IN_LB, 2)


def lb_to_kg(lb: float) -> float:
	return round(lb * KG_IN_LB, 2)


def format_weight(value: float, unit: str) -> str:
	unit = unit.lower()
	if unit not in ("kg", "lb"):
		raise ValueError("unit must be 'kg' or 'lb'")
	return f"{value:.2f} {unit}"
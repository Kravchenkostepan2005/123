from datetime import date, timedelta, datetime
from typing import Tuple


def to_date(value: str) -> date:
	return datetime.strptime(value, "%Y-%m-%d").date()


def to_str(d: date) -> str:
	return d.strftime("%Y-%m-%d")


def week_monday_for(d: date) -> date:
	# Monday is 0
	return d - timedelta(days=d.weekday())


def monday_for_date_str(date_str: str) -> str:
	return to_str(week_monday_for(to_date(date_str)))


def today_monday() -> str:
	return to_str(week_monday_for(date.today()))


def add_days(date_str: str, days: int) -> str:
	return to_str(to_date(date_str) + timedelta(days=days))


def shift_weeks(date_str: str, weeks: int) -> str:
	return add_days(date_str, weeks * 7)


def human_date(date_str: str) -> str:
	d = to_date(date_str)
	return d.strftime("%d.%m.%Y")


def week_range_str(start_monday: str) -> str:
	start = to_date(start_monday)
	end = start + timedelta(days=6)
	return f"{human_date(to_str(start))} — {human_date(to_str(end))}"
from datetime import date, timedelta, datetime
from typing import List, Tuple


KG_PER_LB = 0.45359237


def kg_to_lb(kg: float) -> float:
    return round(kg / KG_PER_LB, 2)


def lb_to_kg(lb: float) -> float:
    return round(lb * KG_PER_LB, 2)


def get_monday(d: date) -> date:
    return d - timedelta(days=d.weekday())


def week_range(start: date) -> List[date]:
    """Возвращает 7 дат недели, начиная с понедельника."""
    monday = get_monday(start)
    return [monday + timedelta(days=i) for i in range(7)]


def period_start(period: str, today: date | None = None) -> date:
    """Возвращает дату начала периода относительно сегодня."""
    if today is None:
        today = date.today()
    period = period.lower()
    if period in ("неделя", "week"):
        return today - timedelta(days=7)
    if period in ("месяц", "month"):
        return today - timedelta(days=30)
    if period in ("3 месяца", "3 months"):
        return today - timedelta(days=90)
    if period in ("6 месяцев", "6 months"):
        return today - timedelta(days=180)
    if period in ("год", "year"):
        return today - timedelta(days=365)
    return today - timedelta(days=30)


def iso_date(d: date | datetime) -> str:
    if isinstance(d, datetime):
        d = d.date()
    return d.isoformat()

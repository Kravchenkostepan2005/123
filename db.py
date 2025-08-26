from peewee import DatabaseProxy, SqliteDatabase
from typing import Sequence, Type

db_proxy: DatabaseProxy = DatabaseProxy()


def initialize_database(db_path: str, models: Sequence[Type]) -> SqliteDatabase:
    """Создает и инициализирует базу данных по указанному пути и таблицы моделей."""
    database = SqliteDatabase(db_path)
    db_proxy.initialize(database)
    database.connect()
    database.create_tables(list(models))
    return database

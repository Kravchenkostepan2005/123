from typing import Generator

from sqlmodel import SQLModel, create_engine, Session

from .config import get_settings


_settings = get_settings()

# For SQLite, we need check_same_thread=False
connect_args = {"check_same_thread": False} if _settings.database_url.startswith("sqlite") else {}
engine = create_engine(_settings.database_url, echo=False, connect_args=connect_args)


def create_db_and_tables() -> None:
    SQLModel.metadata.create_all(engine)


def get_session() -> Generator[Session, None, None]:
    with Session(engine) as session:
        yield session

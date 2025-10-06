from functools import lru_cache
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="WIS2_", env_file=".env", extra="ignore")

    app_name: str = "WIS2"
    secret_key: str = "CHANGE_ME"
    algorithm: str = "HS256"
    access_token_expire_minutes: int = 60 * 24

    # Use file-based SQLite by default
    database_url: str = "sqlite:///./wis2.db"

    # Optional admin seed on startup
    admin_email: str | None = None
    admin_password: str | None = None


@lru_cache()
def get_settings() -> Settings:
    return Settings()

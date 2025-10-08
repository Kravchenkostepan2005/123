from datetime import timedelta

from fastapi import FastAPI

from .core.database import create_db_and_tables, get_session
from .core.config import get_settings
from .api import auth, courses, users, admin, terms
from .core.security import get_password_hash
from .models.user import User
from sqlmodel import select


app = FastAPI(title="WIS2 - Course Management API")


@app.on_event("startup")
def on_startup() -> None:
    # Initialize the database schema
    create_db_and_tables()

    # Optional admin seeding via env
    settings = get_settings()
    if settings.admin_email and settings.admin_password:
        from .core.database import engine
        from sqlmodel import Session

        with Session(engine) as session:
            admin = session.exec(select(User).where(User.email == settings.admin_email)).first()
            if not admin:
                admin = User(
                    email=settings.admin_email,
                    full_name="Administrator",
                    hashed_password=get_password_hash(settings.admin_password),
                    is_admin=True,
                    is_active=True,
                )
                session.add(admin)
                session.commit()


# Routers
app.include_router(auth.router, prefix="/auth", tags=["auth"])
app.include_router(users.router, prefix="/users", tags=["users"])
app.include_router(courses.router, prefix="/courses", tags=["courses"])
app.include_router(admin.router, prefix="/admin", tags=["admin"])
app.include_router(terms.router, prefix="/courses", tags=["terms"])  # nested under /courses/{id}/terms


@app.get("/healthz")
def healthz():
    return {"status": "ok"}

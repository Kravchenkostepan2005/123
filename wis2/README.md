WIS2 - Course management API (FastAPI)

Quickstart

- Create virtualenv and install deps:
```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r wis2/requirements.txt
```

- Run the API:
```bash
export WIS2_SECRET_KEY=dev-secret
uvicorn app.main:app --reload --app-dir wis2
```

- Optional: seed admin via env
```bash
export WIS2_ADMIN_EMAIL=admin@example.com
export WIS2_ADMIN_PASSWORD=admin123
export WIS2_SECRET_KEY=dev-secret
uvicorn app.main:app --reload --app-dir wis2
```

Main endpoints

- Auth
  - POST /auth/register
  - POST /auth/token
- Users
  - GET /users/me
- Courses
  - GET /courses
  - POST /courses
  - GET /courses/{id}
  - POST /courses/{id}/register
  - POST /courses/{id}/instructors/{user_id}
- Admin
  - POST /admin/rooms
  - POST /admin/courses/{id}/approve
  - POST /admin/courses/{id}/reject
 - Terms
   - POST /courses/{course_id}/terms
   - GET /courses/{course_id}/terms
   - POST /courses/terms/{term_id}/register
   - GET /courses/schedule/me
   - POST /courses/terms/{term_id}/grade/{student_id}

Notes

- DB: SQLite file at ./wis2.db (relative to working dir).
- JWT: set WIS2_SECRET_KEY in env for security.
- Models use SQLModel and SQLAlchemy 2.x.
 - Quick test requests in `wis2/scripts/quick_test.http`.

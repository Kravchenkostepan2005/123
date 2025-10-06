from fastapi import APIRouter, Depends, HTTPException
from sqlmodel import Session, select

from ..core.database import get_session
from ..core.security import decode_token
from ..models.user import User

router = APIRouter()


def get_current_user(payload = Depends(decode_token), session: Session = Depends(get_session)) -> User:
    user_id = int(payload.get("sub", 0))
    user = session.get(User, user_id)
    if not user:
        raise HTTPException(status_code=401, detail="User not found")
    return user


@router.get("/me")
def read_me(current_user: User = Depends(get_current_user)):
    return {
        "id": current_user.id,
        "email": current_user.email,
        "full_name": current_user.full_name,
        "is_admin": current_user.is_admin,
        "is_instructor": current_user.is_instructor,
        "is_student": current_user.is_student,
    }

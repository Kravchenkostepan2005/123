from fastapi import APIRouter, Depends, HTTPException
from sqlmodel import Session

from ..core.database import get_session
from ..models.course import Course, CourseStatus, Room
from ..models.user import User
from .users import get_current_user

router = APIRouter()


def require_admin(current_user: User = Depends(get_current_user)) -> User:
    if not current_user.is_admin:
        raise HTTPException(status_code=403, detail="Admin privileges required")
    return current_user


@router.post("/rooms")
def create_room(room: Room, _: User = Depends(require_admin), session: Session = Depends(get_session)):
    session.add(room)
    session.commit()
    session.refresh(room)
    return room


@router.post("/courses/{course_id}/approve")
def approve_course(course_id: int, _: User = Depends(require_admin), session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    course.status = CourseStatus.APPROVED
    session.add(course)
    session.commit()
    return {"status": "approved"}


@router.post("/courses/{course_id}/reject")
def reject_course(course_id: int, _: User = Depends(require_admin), session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    course.status = CourseStatus.REJECTED
    session.add(course)
    session.commit()
    return {"status": "rejected"}

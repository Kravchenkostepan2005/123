from typing import Optional

from fastapi import APIRouter, Depends, HTTPException
from sqlmodel import Session, select

from ..core.database import get_session
from ..models.course import (
    Course,
    CourseStatus,
    CourseRegistration,
    CourseRegistrationStatus,
    CourseInstructor,
)
from ..models.user import User
from .users import get_current_user

router = APIRouter()


@router.get("")
def list_courses(session: Session = Depends(get_session), q: Optional[str] = None):
    query = select(Course).where(Course.status == CourseStatus.APPROVED)
    if q:
        like = f"%{q}%"
        query = query.where((Course.title.ilike(like)) | (Course.code.ilike(like)))
    return session.exec(query).all()


@router.post("")
def create_course(course: Course, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    if not current_user.is_active:
        raise HTTPException(403, "Inactive user")
    course.id = None
    course.owner_id = current_user.id
    course.status = CourseStatus.PENDING_APPROVAL
    session.add(course)
    session.commit()
    session.refresh(course)
    return course


@router.get("/{course_id}")
def get_course(course_id: int, session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    return course


@router.post("/{course_id}/register")
def register_course(course_id: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    existing = session.exec(select(CourseRegistration).where((CourseRegistration.course_id == course_id) & (CourseRegistration.user_id == current_user.id))).first()
    if existing:
        return existing
    registration = CourseRegistration(course_id=course_id, user_id=current_user.id)
    if course.auto_approve_until_limit:
        # Count approvals
        approved_count = session.exec(select(CourseRegistration).where((CourseRegistration.course_id == course_id) & (CourseRegistration.status == CourseRegistrationStatus.APPROVED))).all()
        if course.capacity_limit is None or len(approved_count) < course.capacity_limit:
            registration.status = CourseRegistrationStatus.APPROVED
    session.add(registration)
    session.commit()
    session.refresh(registration)
    return registration


@router.get("/{course_id}/registrations")
def list_registrations(course_id: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    if course.owner_id != current_user.id and not current_user.is_admin:
        raise HTTPException(403, "Only guarantor or admin can view registrations")
    return session.exec(select(CourseRegistration).where(CourseRegistration.course_id == course_id)).all()


@router.post("/{course_id}/registrations/{registration_id}/approve")
def approve_registration(course_id: int, registration_id: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    if course.owner_id != current_user.id and not current_user.is_admin:
        raise HTTPException(403, "Only guarantor or admin can approve")
    reg = session.get(CourseRegistration, registration_id)
    if not reg or reg.course_id != course_id:
        raise HTTPException(404, "Registration not found")
    reg.status = CourseRegistrationStatus.APPROVED
    session.add(reg)
    session.commit()
    return {"status": "approved"}


@router.post("/{course_id}/registrations/{registration_id}/reject")
def reject_registration(course_id: int, registration_id: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    if course.owner_id != current_user.id and not current_user.is_admin:
        raise HTTPException(403, "Only guarantor or admin can reject")
    reg = session.get(CourseRegistration, registration_id)
    if not reg or reg.course_id != course_id:
        raise HTTPException(404, "Registration not found")
    reg.status = CourseRegistrationStatus.REJECTED
    session.add(reg)
    session.commit()
    return {"status": "rejected"}


@router.post("/{course_id}/instructors/{user_id}")
def add_instructor(course_id: int, user_id: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    if course.owner_id != current_user.id and not current_user.is_admin:
        raise HTTPException(403, "Only guarantor or admin can add instructors")
    existing = session.exec(select(CourseInstructor).where((CourseInstructor.course_id == course_id) & (CourseInstructor.user_id == user_id))).first()
    if existing:
        return existing
    ci = CourseInstructor(course_id=course_id, user_id=user_id)
    session.add(ci)
    session.commit()
    session.refresh(ci)
    return ci

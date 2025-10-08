from datetime import datetime
from typing import Optional

from fastapi import APIRouter, Depends, HTTPException
from sqlmodel import Session, select

from ..core.database import get_session
from ..models.course import Course, CourseTerm, TermRegistration, TermGrade
from ..models.user import User
from .users import get_current_user

router = APIRouter()


@router.post("/{course_id}/terms")
def create_term(course_id: int, term: CourseTerm, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    if course.owner_id != current_user.id and not current_user.is_admin:
        raise HTTPException(403, "Only guarantor or admin can add terms")
    term.id = None
    term.course_id = course_id
    session.add(term)
    session.commit()
    session.refresh(term)
    return term


@router.get("/{course_id}/terms")

def list_terms(course_id: int, session: Session = Depends(get_session)):
    return session.exec(select(CourseTerm).where(CourseTerm.course_id == course_id)).all()


@router.post("/terms/{term_id}/register")

def register_for_term(term_id: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    term = session.get(CourseTerm, term_id)
    if not term:
        raise HTTPException(404, "Term not found")
    # If registration is required, check capacity
    if term.requires_registration:
        existing = session.exec(select(TermRegistration).where((TermRegistration.term_id == term_id) & (TermRegistration.user_id == current_user.id))).first()
        if existing:
            return existing
        if term.capacity_limit is not None:
            current = session.exec(select(TermRegistration).where(TermRegistration.term_id == term_id)).all()
            if len(current) >= term.capacity_limit:
                raise HTTPException(400, "Term capacity reached")
        reg = TermRegistration(term_id=term_id, user_id=current_user.id)
        session.add(reg)
        session.commit()
        session.refresh(reg)
        return reg
    # if no registration required, noop
    return {"status": "ok"}


@router.get("/schedule/me")

def my_schedule(current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    # Return terms the user is registered for or all course terms of approved courses the user is registered in
    from ..models.course import CourseRegistration, CourseRegistrationStatus
    course_ids = [cr.course_id for cr in session.exec(select(CourseRegistration).where((CourseRegistration.user_id == current_user.id) & (CourseRegistration.status == CourseRegistrationStatus.APPROVED))).all()]
    terms = session.exec(select(CourseTerm).where(CourseTerm.course_id.in_(course_ids))).all()
    return terms


@router.get("/{course_id}/students")
def list_course_students(course_id: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    # Guarantor or admin only
    course = session.get(Course, course_id)
    if not course:
        raise HTTPException(404, "Course not found")
    if course.owner_id != current_user.id and not current_user.is_admin:
        raise HTTPException(403, "Only guarantor or admin can view students")
    from ..models.course import CourseRegistration, CourseRegistrationStatus
    regs = session.exec(select(CourseRegistration).where((CourseRegistration.course_id == course_id) & (CourseRegistration.status == CourseRegistrationStatus.APPROVED))).all()
    return regs


@router.post("/terms/{term_id}/grade/{student_id}")
def set_term_grade(term_id: int, student_id: int, points: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    # Only course owner or instructor can grade; simplify to owner/admin for MVP
    term = session.get(CourseTerm, term_id)
    if not term:
        raise HTTPException(404, "Term not found")
    course = session.get(Course, term.course_id)
    if course.owner_id != current_user.id and not current_user.is_admin:
        raise HTTPException(403, "Not allowed to grade")
    tg = session.exec(select(TermGrade).where((TermGrade.term_id == term_id) & (TermGrade.student_id == student_id))).first()
    if not tg:
        tg = TermGrade(term_id=term_id, student_id=student_id, points=points)
        session.add(tg)
    else:
        tg.points = points
        session.add(tg)
    session.commit()
    session.refresh(tg)
    return tg


@router.get("/terms/{term_id}/grades")
def list_term_grades(term_id: int, current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    term = session.get(CourseTerm, term_id)
    if not term:
        raise HTTPException(404, "Term not found")
    course = session.get(Course, term.course_id)
    if course.owner_id != current_user.id and not current_user.is_admin:
        raise HTTPException(403, "Not allowed")
    grades = session.exec(select(TermGrade).where(TermGrade.term_id == term_id)).all()
    return grades


@router.get("/grades/me")
def my_grades(current_user: User = Depends(get_current_user), session: Session = Depends(get_session)):
    return session.exec(select(TermGrade).where(TermGrade.student_id == current_user.id)).all()

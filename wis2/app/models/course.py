from __future__ import annotations
from typing import Optional
from datetime import date, datetime
from enum import Enum

from sqlmodel import SQLModel, Field, Relationship


class CourseStatus(str, Enum):
    DRAFT = "draft"
    PENDING_APPROVAL = "pending"
    APPROVED = "approved"
    REJECTED = "rejected"


class Course(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    code: str = Field(index=True, unique=True)
    title: str
    description: str = ""
    type: str = "general"  # lecture, seminar, lab, etc.
    price: float | None = None
    news: str = ""

    owner_id: int = Field(foreign_key="user.id")  # creator; becomes guarantor after approval
    status: CourseStatus = Field(default=CourseStatus.DRAFT)
    auto_approve_until_limit: bool = False
    capacity_limit: int | None = None

    terms: list[CourseTerm] = Relationship(back_populates="course")
    instructors: list[CourseInstructor] = Relationship(back_populates="course")


class TermType(str, Enum):
    LECTURE = "lecture"
    EXERCISE = "exercise"
    EXAM = "exam"
    HOMEWORK = "homework"
    OTHER = "other"


class CourseTerm(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    course_id: int = Field(foreign_key="course.id")
    name: str
    type: TermType = Field(default=TermType.OTHER)
    description: str = ""
    date: datetime
    room_id: int | None = Field(default=None, foreign_key="room.id")
    max_points: int = 100
    requires_registration: bool = False
    capacity_limit: int | None = None

    course: Course = Relationship(back_populates="terms")


class CourseInstructor(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    course_id: int = Field(foreign_key="course.id")
    user_id: int = Field(foreign_key="user.id")

    course: Course = Relationship(back_populates="instructors")


class Room(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    name: str = Field(unique=True, index=True)
    capacity: int | None = None


class CourseRegistrationStatus(str, Enum):
    PENDING = "pending"
    APPROVED = "approved"
    REJECTED = "rejected"


class CourseRegistration(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    course_id: int = Field(foreign_key="course.id")
    user_id: int = Field(foreign_key="user.id")
    status: CourseRegistrationStatus = Field(default=CourseRegistrationStatus.PENDING)
    created_at: datetime = Field(default_factory=datetime.utcnow)


class TermRegistration(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    term_id: int = Field(foreign_key="courseterm.id")
    user_id: int = Field(foreign_key="user.id")
    created_at: datetime = Field(default_factory=datetime.utcnow)


class TermGrade(SQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    term_id: int = Field(foreign_key="courseterm.id")
    student_id: int = Field(foreign_key="user.id")
    points: int = 0

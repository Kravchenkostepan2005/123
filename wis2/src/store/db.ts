import { create } from 'zustand';
import { v4 as uuid } from 'uuid';
import { saveDb, loadDb } from '../utils/storage';
import type {
  WisDatabase,
  Course,
  CourseTerm,
  Enrollment,
  GradeRecord,
  TermRegistration,
  UserProfile,
  Room,
  ApprovalStatus,
} from '../types';
import { generateSeed } from '../utils/seed';

interface DbState extends WisDatabase {
  init: (seedIfEmpty?: boolean) => void;
  upsertCourse: (course: Partial<Course> & { id?: string; guarantorId: string }) => Course;
  approveCourse: (courseId: string, status: ApprovalStatus) => void;
  addLecturer: (courseId: string, lecturerId: string) => void;
  removeLecturer: (courseId: string, lecturerId: string) => void;
  addTerm: (courseId: string, term: Omit<CourseTerm, 'id'>) => CourseTerm;
  updateTerm: (courseId: string, term: CourseTerm) => void;
  deleteTerm: (courseId: string, termId: string) => void;
  enroll: (courseId: string, userId: string) => Enrollment;
  decideEnrollment: (enrollmentId: string, status: Enrollment['status']) => void;
  registerToTerm: (courseId: string, termId: string, userId: string) => TermRegistration;
  gradeStudent: (courseId: string, userId: string, points: number, gradedBy: string, note?: string) => GradeRecord;
  upsertRoom: (room: Partial<Room> & { id?: string }) => Room;
  deleteRoom: (roomId: string) => void;
  updateUserRole: (userId: string, role: UserProfile['role']) => void;
}

export const useDb = create<DbState>((set, get) => ({
  ...((loadDb<WisDatabase>() ?? generateSeed()) as WisDatabase),
  init: (seedIfEmpty = true) => {
    const loaded = loadDb<WisDatabase>();
    if (!loaded && seedIfEmpty) {
      const seed = generateSeed();
      saveDb(seed);
      set(seed);
    } else if (loaded) {
      set(loaded);
    }
  },
  upsertCourse: (course) => {
    const now = new Date().toISOString();
    const db = get();
    let updated: Course;
    if (!course.id) {
      updated = {
        id: uuid(),
        code: course.code ?? 'NEW',
        title: course.title ?? 'New Course',
        type: (course as Course).type ?? 'other',
        description: course.description ?? '',
        price: course.price ?? 0,
        capacity: course.capacity ?? 0,
        autoApproveUntilCapacity: course.autoApproveUntilCapacity ?? false,
        status: 'pending',
        guarantorId: course.guarantorId,
        lecturerIds: [],
        terms: [],
        createdAt: now,
        updatedAt: now,
      };
      const courses = [...db.courses, updated];
      const next = { ...db, courses };
      saveDb(next);
      set(next);
      return updated;
    } else {
      const idx = db.courses.findIndex((c) => c.id === course.id);
      if (idx < 0) throw new Error('Course not found');
      updated = { ...db.courses[idx], ...course, updatedAt: now } as Course;
      const courses = [...db.courses];
      courses[idx] = updated;
      const next = { ...db, courses };
      saveDb(next);
      set(next);
      return updated;
    }
  },
  approveCourse: (courseId, status) => {
    const db = get();
    const idx = db.courses.findIndex((c) => c.id === courseId);
    if (idx < 0) return;
    const courses = [...db.courses];
    courses[idx] = { ...courses[idx], status };
    const next = { ...db, courses };
    saveDb(next);
    set(next);
  },
  addLecturer: (courseId, lecturerId) => {
    const db = get();
    const course = db.courses.find((c) => c.id === courseId);
    if (!course) return;
    if (!course.lecturerIds.includes(lecturerId)) {
      const updated = { ...course, lecturerIds: [...course.lecturerIds, lecturerId] };
      const courses = db.courses.map((c) => (c.id === courseId ? updated : c));
      const next = { ...db, courses };
      saveDb(next);
      set(next);
    }
  },
  removeLecturer: (courseId, lecturerId) => {
    const db = get();
    const course = db.courses.find((c) => c.id === courseId);
    if (!course) return;
    const updated = { ...course, lecturerIds: course.lecturerIds.filter((id) => id !== lecturerId) };
    const courses = db.courses.map((c) => (c.id === courseId ? updated : c));
    const next = { ...db, courses };
    saveDb(next);
    set(next);
  },
  addTerm: (courseId, term) => {
    const db = get();
    const course = db.courses.find((c) => c.id === courseId);
    if (!course) throw new Error('Course not found');
    const newTerm: CourseTerm = { id: uuid(), ...term };
    const updated: Course = { ...course, terms: [...course.terms, newTerm] };
    const courses = db.courses.map((c) => (c.id === courseId ? updated : c));
    const next = { ...db, courses };
    saveDb(next);
    set(next);
    return newTerm;
  },
  updateTerm: (courseId, term) => {
    const db = get();
    const course = db.courses.find((c) => c.id === courseId);
    if (!course) return;
    const updated: Course = {
      ...course,
      terms: course.terms.map((t) => (t.id === term.id ? term : t)),
    };
    const courses = db.courses.map((c) => (c.id === courseId ? updated : c));
    const next = { ...db, courses };
    saveDb(next);
    set(next);
  },
  deleteTerm: (courseId, termId) => {
    const db = get();
    const course = db.courses.find((c) => c.id === courseId);
    if (!course) return;
    const updated: Course = {
      ...course,
      terms: course.terms.filter((t) => t.id !== termId),
    };
    const courses = db.courses.map((c) => (c.id === courseId ? updated : c));
    const next = { ...db, courses };
    saveDb(next);
    set(next);
  },
  enroll: (courseId, userId) => {
    const db = get();
    const existing = db.enrollments.find((e) => e.courseId === courseId && e.userId === userId && e.status !== 'withdrawn');
    if (existing) return existing;
    const now = new Date().toISOString();
    const course = db.courses.find((c) => c.id === courseId);
    if (!course) throw new Error('Course not found');
    const approvedCount = db.enrollments.filter((e) => e.courseId === courseId && e.status === 'approved').length;
    const autoApprove = Boolean(course.autoApproveUntilCapacity && course.capacity && approvedCount < course.capacity);
    const enrollment: Enrollment = {
      id: uuid(),
      courseId,
      userId,
      status: autoApprove ? 'approved' : 'requested',
      requestedAt: now,
      decidedAt: autoApprove ? now : undefined,
    };
    const next = { ...db, enrollments: [...db.enrollments, enrollment] };
    saveDb(next);
    set(next);
    return enrollment;
  },
  decideEnrollment: (enrollmentId, status) => {
    const db = get();
    const enrollments = db.enrollments.map((e) => (e.id === enrollmentId ? { ...e, status, decidedAt: new Date().toISOString() } : e));
    const next = { ...db, enrollments };
    saveDb(next);
    set(next);
  },
  registerToTerm: (courseId, termId, userId) => {
    const db = get();
    const existing = db.termRegistrations.find((r) => r.courseId === courseId && r.termId === termId && r.userId === userId);
    if (existing) return existing;
    const reg: TermRegistration = {
      id: uuid(),
      courseId,
      termId,
      userId,
      registeredAt: new Date().toISOString(),
    };
    const next = { ...db, termRegistrations: [...db.termRegistrations, reg] };
    saveDb(next);
    set(next);
    return reg;
  },
  gradeStudent: (courseId, userId, points, gradedBy, note) => {
    const db = get();
    const rec: GradeRecord = {
      id: uuid(),
      courseId,
      userId,
      points,
      gradedBy,
      gradedAt: new Date().toISOString(),
      note,
    };
    const next = { ...db, grades: [...db.grades, rec] };
    saveDb(next);
    set(next);
    return rec;
  },
  upsertRoom: (room) => {
    const db = get();
    if (!room.id) {
      const created: Room = { id: uuid(), name: room.name ?? 'New Room', capacity: room.capacity ?? 0, location: room.location };
      const next = { ...db, rooms: [...db.rooms, created] };
      saveDb(next);
      set(next);
      return created;
    } else {
      const idx = db.rooms.findIndex((r) => r.id === room.id);
      if (idx < 0) throw new Error('Room not found');
      const rooms = [...db.rooms];
      rooms[idx] = { ...rooms[idx], ...room } as Room;
      const next = { ...db, rooms };
      saveDb(next);
      set(next);
      return rooms[idx];
    }
  },
  deleteRoom: (roomId) => {
    const db = get();
    const next = { ...db, rooms: db.rooms.filter((r) => r.id !== roomId) };
    saveDb(next);
    set(next);
  },
  updateUserRole: (userId, role) => {
    const db = get();
    const users = db.users.map((u) => (u.id === userId ? { ...u, role } : u));
    const next = { ...db, users };
    saveDb(next);
    set(next);
  },
}));

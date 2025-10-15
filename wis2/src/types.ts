export type Role = 'guest' | 'registered' | 'admin';

export type CourseType = 'programming' | 'math' | 'languages' | 'design' | 'business' | 'other';

export type TermType = 'lecture' | 'exercise' | 'exam' | 'homework' | 'project' | 'other';

export type ApprovalStatus = 'pending' | 'approved' | 'rejected';

export interface Room {
  id: string;
  name: string;
  capacity: number;
  location?: string;
}

export interface UserProfile {
  id: string;
  email: string;
  fullName: string;
  role: Role; // global role
}

export interface CourseTerm {
  id: string;
  title: string;
  type: TermType;
  description?: string;
  date: string; // ISO 8601
  roomId?: string;
  requiresRegistration?: boolean;
}

export interface Course {
  id: string;
  code: string; // short identifier
  title: string;
  type: CourseType;
  description?: string;
  price?: number;
  capacity?: number; // student limit
  autoApproveUntilCapacity?: boolean;
  status: ApprovalStatus;
  guarantorId: string; // creator/owner
  lecturerIds: string[]; // users teaching
  terms: CourseTerm[];
  createdAt: string; // ISO 8601
  updatedAt: string; // ISO 8601
}

export interface Enrollment {
  id: string;
  courseId: string;
  userId: string; // student
  status: 'requested' | 'approved' | 'rejected' | 'withdrawn';
  requestedAt: string;
  decidedAt?: string;
}

export interface TermRegistration {
  id: string;
  termId: string;
  courseId: string;
  userId: string; // student
  registeredAt: string;
}

export interface GradeRecord {
  id: string;
  courseId: string;
  userId: string; // student
  points: number; // 0-100
  gradedBy: string; // lecturerId
  gradedAt: string;
  note?: string;
}

export interface WisDatabase {
  users: UserProfile[];
  rooms: Room[];
  courses: Course[];
  enrollments: Enrollment[];
  termRegistrations: TermRegistration[];
  grades: GradeRecord[];
}

export interface ScheduleEntry {
  term: CourseTerm;
  course: Course;
}

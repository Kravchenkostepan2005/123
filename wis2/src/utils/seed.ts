import { v4 as uuid } from 'uuid';
import { addDays } from 'date-fns';
import type { Course, CourseTerm, UserProfile, Room, WisDatabase } from '../types';

export function generateSeed(): WisDatabase {
  const now = new Date();

  const users: UserProfile[] = [
    { id: uuid(), email: 'admin@example.com', fullName: 'Admin User', role: 'admin' },
    { id: uuid(), email: 'alice@example.com', fullName: 'Alice Registered', role: 'registered' },
    { id: uuid(), email: 'bob@example.com', fullName: 'Bob Registered', role: 'registered' },
    { id: uuid(), email: 'carol@example.com', fullName: 'Carol Lecturer', role: 'registered' },
  ];

  const [_, alice, bob, carol] = users;

  const rooms: Room[] = [
    { id: uuid(), name: 'A-101', capacity: 40, location: 'Building A' },
    { id: uuid(), name: 'B-202', capacity: 20, location: 'Building B' },
  ];

  const course1Terms: CourseTerm[] = [
    {
      id: uuid(),
      title: 'Intro Lecture',
      type: 'lecture',
      date: addDays(now, 3).toISOString(),
      description: 'Overview of the course',
      roomId: rooms[0].id,
      requiresRegistration: false,
    },
    {
      id: uuid(),
      title: 'Exercise 1',
      type: 'exercise',
      date: addDays(now, 5).toISOString(),
      roomId: rooms[1].id,
      requiresRegistration: true,
    },
  ];

  const courses: Course[] = [
    {
      id: uuid(),
      code: 'JS101',
      title: 'JavaScript for Beginners',
      type: 'programming',
      description: 'Learn the basics of JavaScript',
      price: 0,
      capacity: 30,
      autoApproveUntilCapacity: true,
      status: 'approved',
      guarantorId: alice.id,
      lecturerIds: [carol.id],
      terms: course1Terms,
      createdAt: now.toISOString(),
      updatedAt: now.toISOString(),
    },
    {
      id: uuid(),
      code: 'MATH201',
      title: 'Linear Algebra',
      type: 'math',
      description: 'Vectors, matrices, eigen-things',
      price: 100,
      capacity: 25,
      autoApproveUntilCapacity: false,
      status: 'pending',
      guarantorId: bob.id,
      lecturerIds: [],
      terms: [],
      createdAt: now.toISOString(),
      updatedAt: now.toISOString(),
    },
  ];

  return {
    users,
    rooms,
    courses,
    enrollments: [],
    termRegistrations: [],
    grades: [],
  } as WisDatabase;
}

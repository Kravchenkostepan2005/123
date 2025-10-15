export type UserRole = 'admin' | 'garant' | 'lecturer' | 'student' | 'guest';

export type CourseType = 'přednáška' | 'seminář' | 'workshop' | 'online';

export interface CourseTerm {
  id: string;
  name: string;
  type: 'přednáška' | 'cvičení' | 'zkouška' | 'úkol';
  description?: string;
  date: string; // ISO
  room?: string;
}

export interface Course {
  id: string;
  code: string;
  title: string;
  description: string;
  type: CourseType;
  price: number;
  terms: CourseTerm[];
  instructors: string[];
  garant: string;
}

export const courses: Course[] = [
  {
    id: 'c1',
    code: 'IIS101',
    title: 'Úvod do informačních systémů',
    description: 'Základy návrhu a implementace IS, procesy, modelování a architektury.',
    type: 'přednáška',
    price: 0,
    terms: [
      { id: 't1', name: 'Přednáška 1', type: 'přednáška', date: '2025-10-20T10:00:00' },
      { id: 't2', name: 'Cvičení 1', type: 'cvičení', date: '2025-10-24T14:00:00', room: 'D105' },
    ],
    instructors: ['doc. Novák', 'Ing. Svoboda'],
    garant: 'Ing. Hynek',
  },
  {
    id: 'c2',
    code: 'DB201',
    title: 'Databázové systémy',
    description: 'Relační model, SQL, normalizace, transakce a návrh databází.',
    type: 'seminář',
    price: 500,
    terms: [
      { id: 't3', name: 'Přednáška 1', type: 'přednáška', date: '2025-10-22T12:00:00', room: 'A101' },
      { id: 't4', name: 'Zkouška', type: 'zkouška', date: '2025-12-10T09:00:00', room: 'B203' },
    ],
    instructors: ['Mgr. Dvořák'],
    garant: 'Ing. Hynek',
  },
];

import { courses as seedCourses } from '../mocks/data';
import { loadFromStorage, saveToStorage } from './storage';

const STORAGE_KEY = 'wis2:courses';

export function getAllCourses() {
  return loadFromStorage(STORAGE_KEY, seedCourses);
}

export function upsertCourse(course: (typeof seedCourses)[number]) {
  const all = getAllCourses();
  const idx = all.findIndex((c) => c.id === course.id);
  if (idx >= 0) {
    all[idx] = course as any;
  } else {
    all.push(course as any);
  }
  saveToStorage(STORAGE_KEY, all);
  return course;
}

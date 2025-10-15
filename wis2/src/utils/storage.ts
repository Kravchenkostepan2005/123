const STORAGE_KEY = 'wis2_db_v1';
const USER_KEY = 'wis2_user_v1';

export function load<T>(key: string): T | null {
  if (typeof localStorage === 'undefined') return null as unknown as T;
  try {
    const raw = localStorage.getItem(key);
    if (!raw) return null;
    return JSON.parse(raw) as T;
  } catch {
    return null as unknown as T;
  }
}

export function save<T>(key: string, value: T): void {
  if (typeof localStorage === 'undefined') return;
  localStorage.setItem(key, JSON.stringify(value));
}

export function loadDb<T>(): T | null {
  return load<T>(STORAGE_KEY);
}

export function saveDb<T>(db: T): void {
  save<T>(STORAGE_KEY, db);
}

export function loadUser<T>(): T | null {
  return load<T>(USER_KEY);
}

export function saveUser<T>(user: T): void {
  save<T>(USER_KEY, user);
}

export function clearAll(): void {
  if (typeof localStorage === 'undefined') return;
  localStorage.removeItem(STORAGE_KEY);
  localStorage.removeItem(USER_KEY);
}

export const keys = { STORAGE_KEY, USER_KEY } as const;

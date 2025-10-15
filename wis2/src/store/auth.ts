import { create } from 'zustand';
import type { UserProfile } from '../types';
import { loadUser, saveUser } from '../utils/storage';

interface AuthState {
  user: UserProfile | null;
  users: UserProfile[]; // for the simple demo picker
  setUsers: (users: UserProfile[]) => void;
  loginAs: (user: UserProfile) => void;
  logout: () => void;
}

export const useAuthStore = create<AuthState>((set) => ({
  user: loadUser<UserProfile>() ?? null,
  users: [],
  setUsers: (users) => set({ users }),
  loginAs: (user) => {
    saveUser(user);
    set({ user });
  },
  logout: () => {
    saveUser<UserProfile | null>(null as unknown as UserProfile | null);
    set({ user: null });
  },
}));

import { createContext, useContext, useMemo, useState } from 'react';

export type Role = 'guest' | 'student' | 'lecturer' | 'garant' | 'admin';

export interface UserInfo {
  id: string;
  name: string;
  role: Role;
}

interface AuthContextValue {
  user: UserInfo | null;
  loginAs: (role: Role) => void;
  logout: () => void;
}

const AuthContext = createContext<AuthContextValue | undefined>(undefined);

export function AuthProvider({ children }: { children: React.ReactNode }) {
  const [user, setUser] = useState<UserInfo | null>(null);

  const value = useMemo<AuthContextValue>(
    () => ({
      user,
      loginAs: (role) => setUser({ id: 'u1', name: 'Demo User', role }),
      logout: () => setUser(null),
    }),
    [user]
  );

  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>;
}

export function useAuth() {
  const ctx = useContext(AuthContext);
  if (!ctx) throw new Error('useAuth must be used within AuthProvider');
  return ctx;
}

import { Link, Outlet } from 'react-router-dom';

import { useAuth } from '../context/AuthContext';

export default function AppLayout() {
  const { user } = useAuth();
  return (
    <div className="min-h-screen bg-gray-50 text-gray-900">
      <header className="sticky top-0 z-10 bg-white border-b">
        <div className="max-w-7xl mx-auto flex items-center justify-between px-4 py-3">
          <Link to="/" className="text-xl font-semibold text-brand-700">WIS2</Link>
          <nav className="flex items-center gap-4 text-sm">
            <Link to="/catalog" className="hover:text-brand-600">Katalog kurzů</Link>
            <Link to="/dashboard" className="hover:text-brand-600">Můj přehled</Link>
            {user?.role === 'admin' && (
              <Link to="/admin" className="hover:text-brand-600">Admin</Link>
            )}
          </nav>
        </div>
      </header>
      <main className="max-w-7xl mx-auto px-4 py-6">
        <Outlet />
      </main>
      <footer className="border-t bg-white">
        <div className="max-w-7xl mx-auto px-4 py-6 text-sm text-gray-500">
          © {new Date().getFullYear()} WIS2 — jednoduchý IS pro kurzy
        </div>
      </footer>
    </div>
  );
}

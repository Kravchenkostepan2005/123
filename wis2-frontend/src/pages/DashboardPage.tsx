import { useAuth } from '../context/AuthContext';

export default function DashboardPage() {
  const { user, loginAs, logout } = useAuth();

  return (
    <section className="space-y-6">
      <header className="flex items-center justify-between">
        <h1 className="text-2xl font-bold">Můj přehled</h1>
        <div className="flex items-center gap-2">
          {!user ? (
            <div className="flex items-center gap-2">
              <button className="border rounded px-3 py-1" onClick={() => loginAs('student')}>Přihlásit jako student</button>
              <button className="border rounded px-3 py-1" onClick={() => loginAs('lecturer')}>Lektor</button>
              <button className="border rounded px-3 py-1" onClick={() => loginAs('garant')}>Garant</button>
              <button className="border rounded px-3 py-1" onClick={() => loginAs('admin')}>Admin</button>
            </div>
          ) : (
            <div className="flex items-center gap-2">
              <span className="text-sm text-gray-700">{user.name} ({user.role})</span>
              <button className="border rounded px-3 py-1" onClick={logout}>Odhlásit</button>
            </div>
          )}
        </div>
      </header>

      {!user && (
        <p className="text-gray-600">Pro zobrazení personalizovaného obsahu se přihlaste jednou z rolí.</p>
      )}

      {user?.role === 'student' && (
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
          <div className="bg-white border rounded p-4">
            <h3 className="font-semibold mb-2">Moje kurzy</h3>
            <ul className="list-disc list-inside text-sm text-gray-700">
              <li>Úvod do IS</li>
              <li>Databáze</li>
            </ul>
          </div>
          <div className="bg-white border rounded p-4">
            <h3 className="font-semibold mb-2">Nadcházející termíny</h3>
            <ul className="text-sm text-gray-700 space-y-1">
              <li>20.10. 10:00 — Přednáška: Úvod do IS</li>
              <li>24.10. 14:00 — Cvičení: ER diagramy</li>
            </ul>
          </div>
          <div className="bg-white border rounded p-4">
            <h3 className="font-semibold mb-2">Získané body</h3>
            <p className="text-2xl font-semibold">72 / 100</p>
          </div>
        </div>
      )}

      {(user?.role === 'lecturer' || user?.role === 'garant') && (
        <div className="space-y-4">
          <div className="bg-white border rounded p-4">
            <h3 className="font-semibold mb-2">Správa kurzů</h3>
            <p className="text-sm text-gray-700">Zakládejte kurzy, upravujte parametry, plánujte termíny, hodnoťte studenty.</p>
            <div className="mt-3 flex gap-2">
              <button className="bg-brand-600 text-white rounded px-3 py-2">Nový kurz</button>
              <button className="border rounded px-3 py-2">Moje kurzy</button>
            </div>
          </div>
          <div className="bg-white border rounded p-4">
            <h3 className="font-semibold mb-2">Schvalování registrací</h3>
            <p className="text-sm text-gray-700">Nastavte limit a automatické potvrzování do naplnění kapacity.</p>
            <button className="mt-3 border rounded px-3 py-2">Otevřít</button>
          </div>
        </div>
      )}

      {user?.role === 'admin' && (
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
          <div className="bg-white border rounded p-4">
            <h3 className="font-semibold mb-2">Schvalování kurzů</h3>
            <p className="text-sm text-gray-600">3 čekající žádosti</p>
            <button className="mt-3 bg-brand-600 text-white rounded px-3 py-2">Otevřít</button>
          </div>
          <div className="bg-white border rounded p-4">
            <h3 className="font-semibold mb-2">Uživatelé</h3>
            <p className="text-sm text-gray-600">Přehled a správa rolí</p>
            <button className="mt-3 bg-brand-600 text-white rounded px-3 py-2">Otevřít</button>
          </div>
          <div className="bg-white border rounded p-4">
            <h3 className="font-semibold mb-2">Místnosti</h3>
            <p className="text-sm text-gray-600">Definice místností a kapacity</p>
            <button className="mt-3 bg-brand-600 text-white rounded px-3 py-2">Otevřít</button>
          </div>
        </div>
      )}
    </section>
  );
}

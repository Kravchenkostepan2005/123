export default function AdminPage() {
  return (
    <section className="space-y-6">
      <h1 className="text-2xl font-bold">Administrace</h1>
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
    </section>
  );
}

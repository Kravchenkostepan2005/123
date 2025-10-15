export default function HomePage() {
  return (
    <section className="space-y-6">
      <h1 className="text-3xl font-bold">Vítejte ve WIS2</h1>
      <p className="text-gray-600 max-w-2xl">
        Jednoduchý informační systém pro správu a registraci výukových kurzů. Prohlédněte si nabídku
        kurzů v katalogu, přihlaste se a spravujte své kurzy, rozvrh a hodnocení.
      </p>
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4">
        <a className="p-4 bg-white border rounded-lg shadow-sm hover:shadow transition" href="/catalog">
          <h3 className="font-semibold mb-1">Katalog kurzů</h3>
          <p className="text-sm text-gray-600">Veřejný přehled dostupných kurzů.</p>
        </a>
        <a className="p-4 bg-white border rounded-lg shadow-sm hover:shadow transition" href="/dashboard">
          <h3 className="font-semibold mb-1">Můj přehled</h3>
          <p className="text-sm text-gray-600">Moje kurzy, rozvrh a hodnocení.</p>
        </a>
        <a className="p-4 bg-white border rounded-lg shadow-sm hover:shadow transition" href="/admin">
          <h3 className="font-semibold mb-1">Administrace</h3>
          <p className="text-sm text-gray-600">Správa uživatelů, místností a schvalování kurzů.</p>
        </a>
      </div>
    </section>
  );
}

import { useState, useMemo } from 'react';
import { courses } from '../mocks/data';

export default function CatalogPage() {
  const [query, setQuery] = useState('');
  const filtered = useMemo(
    () =>
      courses.filter((c) =>
        [c.title, c.code, c.type, c.description]
          .join(' ')
          .toLowerCase()
          .includes(query.toLowerCase())
      ),
    [query]
  );

  return (
    <section className="space-y-4">
      <div className="flex items-center justify-between gap-4">
        <h1 className="text-2xl font-bold">Katalog kurzů</h1>
        <input
          className="border rounded px-3 py-2 w-72"
          placeholder="Hledat kurzy..."
          value={query}
          onChange={(e) => setQuery(e.target.value)}
        />
      </div>
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        {filtered.map((c) => (
          <article key={c.id} className="bg-white border rounded-lg p-4 shadow-sm">
            <h3 className="font-semibold text-lg">{c.title}</h3>
            <p className="text-sm text-gray-600 mb-3">{c.description}</p>
            <div className="flex items-center justify-between text-sm">
              <span className="text-gray-700">{c.type} · {c.price} Kč</span>
              <a href={`/catalog/${c.id}`} className="text-brand-700 hover:underline">Detail</a>
            </div>
          </article>
        ))}
      </div>
    </section>
  );
}

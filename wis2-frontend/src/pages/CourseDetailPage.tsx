import { useMemo } from 'react';
import { useParams } from 'react-router-dom';
import { courses } from '../mocks/data';
import { format } from 'date-fns';

export default function CourseDetailPage() {
  const { courseId } = useParams();
  const course = useMemo(() => courses.find((c) => c.id === courseId), [courseId]);

  if (!course) return <p className="text-gray-600">Kurz nenalezen.</p>;

  return (
    <section className="space-y-6">
      <header className="flex items-start justify-between gap-4">
        <div>
          <h1 className="text-3xl font-bold">{course.title}</h1>
          <p className="text-gray-600">{course.code} · {course.type} · {course.price} Kč</p>
        </div>
        <button className="bg-brand-600 text-white rounded px-4 py-2">Registrovat se</button>
      </header>

      <article className="prose max-w-none">
        <p>{course.description}</p>
      </article>

      <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
        <div className="md:col-span-2 bg-white border rounded p-4">
          <h3 className="font-semibold mb-3">Termíny</h3>
          <ul className="divide-y">
            {course.terms.map((t) => (
              <li key={t.id} className="py-3 flex items-center justify-between">
                <div>
                  <p className="font-medium">{t.name} <span className="text-xs text-gray-500">({t.type})</span></p>
                  <p className="text-sm text-gray-600">{t.room ? `Místnost ${t.room} · ` : ''}{format(new Date(t.date), 'd.M.yyyy HH:mm')}</p>
                </div>
                <button className="text-brand-700 hover:underline text-sm">Registrovat</button>
              </li>
            ))}
          </ul>
        </div>
        <aside className="bg-white border rounded p-4">
          <h3 className="font-semibold mb-3">Lektori a garant</h3>
          <p className="text-sm text-gray-700 mb-2">Garant: {course.garant}</p>
          <ul className="list-disc list-inside text-sm text-gray-700">
            {course.instructors.map((i) => (
              <li key={i}>{i}</li>
            ))}
          </ul>
        </aside>
      </div>
    </section>
  );
}

## WIS2 — Jednoduchý IS pro kurzy

Frontend postavený na React + Vite + TypeScript + Tailwind CSS. Implementuje základní rozhraní dle zadání:

- veřejný katalog kurzů s vyhledáváním
- detail kurzu (termíny, lektoři, garant, registrace)
- dashboard pro role student/lektor/garant/admin (demo přepínání rolí)
- jednoduchá mock data a perzistence v `localStorage`

### Požadavky
- Node.js 18+

### Instalace a spuštění
```bash
npm install
npm run dev
```

### Build
```bash
npm run build
npm run preview
```

### Struktura
- `src/pages` — stránky: katalog, detail, dashboard, admin
- `src/layouts` — hlavní layout aplikace
- `src/routes` — router konfigurace
- `src/context` — kontext autentizace a rolí
- `src/mocks` — ukázková data
- `src/services` — přístup k datům (localStorage)

### Poznámky k rozšíření
- Přidat reálné API a perzistenci (REST/GraphQL)
- Model registrací studentů, schvalování, limity kapacity
- Hodnocení po termínech a součtové skóre 0–100 bodů
- Správa místností a rozvrhové konflikty
- Detailnější oprávnění a audit logy

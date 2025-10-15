import { useMemo, useState } from 'react';
import { Box, Card, CardActionArea, CardContent, Chip, Stack, TextField, Typography } from '@mui/material';
import { Link } from 'react-router-dom';
import { useDb } from '../../store/db';

export default function Catalog() {
  const courses = useDb((s) => s.courses);
  const [query, setQuery] = useState('');
  const filtered = useMemo(() => courses.filter((c) => c.status === 'approved' && (c.title.toLowerCase().includes(query.toLowerCase()) || c.code.toLowerCase().includes(query.toLowerCase()))), [courses, query]);

  return (
    <Stack spacing={2}>
      <Typography variant="h5">Dostupné kurzy</Typography>
      <TextField label="Hledat" value={query} onChange={(e) => setQuery(e.target.value)} />
      <Box sx={{ display: 'grid', gridTemplateColumns: { xs: '1fr', md: '1fr 1fr', lg: 'repeat(3, 1fr)' }, gap: 2 }}>
        {filtered.map((c) => (
          <Card key={c.id}>
            <CardActionArea component={Link} to={`/course/${c.id}`}>
              <CardContent>
                <Stack spacing={1}>
                  <Typography variant="h6">{c.title}</Typography>
                  <Typography variant="body2" color="text.secondary">{c.code}</Typography>
                  <Box>
                    <Chip label={c.type} size="small" />
                    {typeof c.price === 'number' && <Chip sx={{ ml: 1 }} label={`Cena: ${c.price}`} size="small" />}
                  </Box>
                  {c.description && <Typography variant="body2">{c.description}</Typography>}
                </Stack>
              </CardContent>
            </CardActionArea>
          </Card>
        ))}
      </Box>
    </Stack>
  );
}

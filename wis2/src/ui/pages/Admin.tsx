import { useState } from 'react';
import { Box, Button, Card, CardActions, CardContent, Divider, MenuItem, Select, Stack, TextField, Typography } from '@mui/material';
import { useDb } from '../../store/db';

export default function Admin() {
  const db = useDb();
  const [roomName, setRoomName] = useState('');
  const [roomCapacity, setRoomCapacity] = useState<number>(20);

  const pending = db.courses.filter((c) => c.status === 'pending');

  return (
    <Stack spacing={2}>
      <Typography variant="h5">Administrace</Typography>

      <Typography variant="h6">Schvalování kurzů</Typography>
      <Box sx={{ display: 'grid', gridTemplateColumns: { xs: '1fr', md: '1fr 1fr' }, gap: 2 }}>
        {pending.map((c) => (
          <Card key={c.id}>
            <CardContent>
              <Typography variant="h6">{c.title}</Typography>
              <Typography color="text.secondary">{c.code}</Typography>
            </CardContent>
            <CardActions>
              <Button onClick={() => db.approveCourse(c.id, 'approved')} variant="contained">Schválit</Button>
              <Button onClick={() => db.approveCourse(c.id, 'rejected')} color="error" variant="outlined">Zamítnout</Button>
            </CardActions>
          </Card>
        ))}
      </Box>

      <Divider />
      <Typography variant="h6">Místnosti</Typography>
      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={2}>
        <TextField label="Název" value={roomName} onChange={(e) => setRoomName(e.target.value)} />
        <TextField label="Kapacita" type="number" value={roomCapacity} onChange={(e) => setRoomCapacity(Number(e.target.value))} />
        <Button variant="contained" onClick={() => {
          if (!roomName) return;
          db.upsertRoom({ name: roomName, capacity: roomCapacity });
          setRoomName('');
          setRoomCapacity(20);
        }}>Přidat místnost</Button>
      </Stack>
      <Stack spacing={1}>
        {db.rooms.map((r) => (
          <Card key={r.id}><CardContent><Typography>{r.name} — kapacita {r.capacity}</Typography></CardContent></Card>
        ))}
      </Stack>

      <Divider />
      <Typography variant="h6">Uživatelé</Typography>
      <Stack spacing={1}>
        {db.users.map((u) => (
          <Card key={u.id}>
            <CardContent>
              <Stack direction={{ xs: 'column', sm: 'row' }} spacing={2} alignItems="center">
                <Box sx={{ flexGrow: 1 }}>
                  <Typography>{u.fullName}</Typography>
                  <Typography color="text.secondary">{u.email}</Typography>
                </Box>
                <Select size="small" value={u.role} onChange={(e) => db.updateUserRole(u.id, e.target.value as any)}>
                  <MenuItem value="registered">registered</MenuItem>
                  <MenuItem value="admin">admin</MenuItem>
                </Select>
              </Stack>
            </CardContent>
          </Card>
        ))}
      </Stack>
    </Stack>
  );
}

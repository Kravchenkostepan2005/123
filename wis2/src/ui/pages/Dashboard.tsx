import { useState } from 'react';
import { Box, Button, Card, CardContent, Divider, MenuItem, Select, Stack, Tab, Tabs, TextField, Typography } from '@mui/material';
import { DateTimePicker, LocalizationProvider } from '@mui/x-date-pickers';
import { AdapterDateFns } from '@mui/x-date-pickers/AdapterDateFns';
import { useAuthStore } from '../../store/auth';
import { useDb } from '../../store/db';
import type { Course, CourseTerm } from '../../types';

export default function Dashboard() {
  const user = useAuthStore((s) => s.user);
  const [tab, setTab] = useState(0);

  if (!user) return <Typography>Přihlaste se.</Typography>;

  // Role-based tabs: student, lecturer, guarantor
  return (
    <Stack spacing={2}>
      <Typography variant="h5">Můj přehled</Typography>
      <Tabs value={tab} onChange={(_, v) => setTab(v)}>
        <Tab label="Student" />
        <Tab label="Lektor" />
        <Tab label="Garant" />
      </Tabs>
      {tab === 0 && <StudentTab />}
      {tab === 1 && <LecturerTab />}
      {tab === 2 && <GuarantorTab />}
    </Stack>
  );
}

function StudentTab() {
  const user = useAuthStore((s) => s.user)!;
  const db = useDb();
  const myEnrollments = db.enrollments.filter((e) => e.userId === user.id && e.status === 'approved');
  const myCourses = myEnrollments.map((e) => db.courses.find((c) => c.id === e.courseId)).filter(Boolean) as Course[];
  const myTerms = myCourses.flatMap((c) => c.terms.map((t) => ({ course: c, term: t })));
  return (
    <Stack spacing={2}>
      <Typography variant="h6">Moje kurzy</Typography>
      {myCourses.map((c) => (
        <Card key={c.id}><CardContent><Typography>{c.title}</Typography></CardContent></Card>
      ))}
      <Divider />
      <Typography variant="h6">Rozvrh (termíny)</Typography>
      <Stack spacing={1}>
        {myTerms.map(({ course, term }) => {
          const already = db.termRegistrations.find((r) => r.courseId === course.id && r.termId === term.id && r.userId === user.id);
          return (
            <Card key={term.id}>
              <CardContent>
                <Stack direction={{ xs: 'column', sm: 'row' }} spacing={2} alignItems="center">
                  <Box sx={{ flexGrow: 1 }}>
                    <Typography>{course.title} — {term.title} ({term.type})</Typography>
                    <Typography color="text.secondary">{new Date(term.date).toLocaleString()}</Typography>
                  </Box>
                  {term.requiresRegistration && !already ? (
                    <Button onClick={() => db.registerToTerm(course.id, term.id, user.id)} variant="outlined">Registrovat na termín</Button>
                  ) : (
                    <Typography color="success.main">{term.requiresRegistration ? 'Registrován' : 'Není vyžadována registrace'}</Typography>
                  )}
                </Stack>
              </CardContent>
            </Card>
          );
        })}
      </Stack>
    </Stack>
  );
}

function LecturerTab() {
  const user = useAuthStore((s) => s.user)!;
  const db = useDb();
  const myCourses = db.courses.filter((c) => c.lecturerIds.includes(user.id));
  return (
    <Stack spacing={2}>
      <Typography variant="h6">Kurzy, kde učím</Typography>
      {myCourses.map((c) => (
        <CourseGrading key={c.id} courseId={c.id} />
      ))}
    </Stack>
  );
}

function CourseGrading({ courseId }: { courseId: string }) {
  const db = useDb();
  const user = useAuthStore((s) => s.user)!;
  const course = db.courses.find((c) => c.id === courseId)!;
  const students = db.enrollments.filter((e) => e.courseId === courseId && e.status === 'approved');
  const [pointsByStudent, setPointsByStudent] = useState<Record<string, number>>({});
  return (
    <Card>
      <CardContent>
        <Typography variant="h6">{course.title}</Typography>
        <Stack spacing={1} sx={{ mt: 1 }}>
          {students.length === 0 && <Typography color="text.secondary">Žádní studenti</Typography>}
          {students.map((e) => {
            const student = db.users.find((u) => u.id === e.userId)!;
            const lastGrade = [...db.grades].reverse().find((g) => g.courseId === courseId && g.userId === e.userId);
            return (
              <Stack key={e.id} direction={{ xs: 'column', sm: 'row' }} spacing={2} alignItems="center">
                <Box sx={{ flexGrow: 1 }}>
                  <Typography>{student.fullName}</Typography>
                  {lastGrade && <Typography color="text.secondary">Poslední známka: {lastGrade.points}</Typography>}
                </Box>
                <TextField
                  size="small"
                  type="number"
                  label="Body (0-100)"
                  value={pointsByStudent[e.userId] ?? ''}
                  onChange={(ev) => setPointsByStudent({ ...pointsByStudent, [e.userId]: Number(ev.target.value) })}
                />
                <Button variant="contained" onClick={() => {
                  const val = pointsByStudent[e.userId];
                  if (typeof val !== 'number' || isNaN(val)) return;
                  db.gradeStudent(courseId, e.userId, Math.max(0, Math.min(100, val)), user.id);
                }}>Uložit</Button>
              </Stack>
            );
          })}
        </Stack>
      </CardContent>
    </Card>
  );
}

function GuarantorTab() {
  const user = useAuthStore((s) => s.user)!;
  const db = useDb();
  const [draft, setDraft] = useState<Partial<Course>>({ title: '', code: '', type: 'other', description: '' });
  const [selectedCourseId, setSelectedCourseId] = useState<string>('');
  // state handled in CourseManager

  const myCourses = db.courses.filter((c) => c.guarantorId === user.id);

  return (
    <Stack spacing={2}>
      <Typography variant="h6">Moje kurzy (garant)</Typography>
      <Select size="small" value={selectedCourseId} onChange={(e) => setSelectedCourseId(e.target.value)} displayEmpty>
        <MenuItem value=""><em>Vyberte kurz k editaci</em></MenuItem>
        {myCourses.map((c) => (<MenuItem key={c.id} value={c.id}>{c.title}</MenuItem>))}
      </Select>
      {selectedCourseId && <CourseManager courseId={selectedCourseId} />}
      <Divider />
      <Typography variant="h6">Založit nový kurz</Typography>
      <Stack direction="row" spacing={2}>
        <TextField label="Název" value={draft.title} onChange={(e) => setDraft({ ...draft, title: e.target.value })} />
        <TextField label="Kód" value={draft.code} onChange={(e) => setDraft({ ...draft, code: e.target.value })} />
        <TextField label="Typ" value={draft.type} onChange={(e) => setDraft({ ...draft, type: e.target.value as any })} />
        <Button variant="contained" onClick={() => {
          if (!draft.title || !draft.code) return;
          db.upsertCourse({ ...draft, guarantorId: user.id } as any);
          setDraft({ title: '', code: '', type: 'other', description: '' });
        }}>Vytvořit</Button>
      </Stack>
    </Stack>
  );
}

function CourseManager({ courseId }: { courseId: string }) {
  const db = useDb();
  const course = db.courses.find((c) => c.id === courseId)!;
  const [capacity, setCapacity] = useState<number>(course.capacity ?? 0);
  const [autoApprove, setAutoApprove] = useState<boolean>(Boolean(course.autoApproveUntilCapacity));
  const [termDraftTitle, setTermDraftTitle] = useState('');
  const [termDraftType, setTermDraftType] = useState<CourseTerm['type']>('lecture');
  const [termDate, setTermDate] = useState<Date | null>(new Date());
  const [termRoom, setTermRoom] = useState<string>('');
  const [termRequires, setTermRequires] = useState<boolean>(false);
  const [lecturerId, setLecturerId] = useState<string>('');

  const pendingEnrollments = db.enrollments.filter((e) => e.courseId === courseId && e.status === 'requested');

  return (
    <Stack spacing={2}>
      <Typography variant="h6">Správa kurzu: {course.title}</Typography>
      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={2}>
        <TextField label="Kapacita" type="number" value={capacity} onChange={(e) => setCapacity(Number(e.target.value))} />
        <Select size="small" value={String(autoApprove)} onChange={(e) => setAutoApprove(e.target.value === 'true')}>
          <MenuItem value="false">Ruční schvalování</MenuItem>
          <MenuItem value="true">Auto do kapacity</MenuItem>
        </Select>
        <Button variant="outlined" onClick={() => db.upsertCourse({ id: courseId, capacity, autoApproveUntilCapacity: autoApprove, guarantorId: course.guarantorId })}>Uložit parametry</Button>
      </Stack>

      <Divider />
      <Typography variant="subtitle1">Lektori</Typography>
      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={2} alignItems="center">
        <Select size="small" value={lecturerId} onChange={(e) => setLecturerId(e.target.value)} displayEmpty>
          <MenuItem value=""><em>Vybrat uživatele</em></MenuItem>
          {db.users.map((u) => (<MenuItem key={u.id} value={u.id}>{u.fullName}</MenuItem>))}
        </Select>
        <Button onClick={() => { if (lecturerId) db.addLecturer(courseId, lecturerId); }} variant="outlined">Přidat lektora</Button>
      </Stack>
      <Stack spacing={1}>
        {course.lecturerIds.map((id) => {
          const u = db.users.find((x) => x.id === id);
          if (!u) return null;
          return <Typography key={id}>• {u.fullName}</Typography>;
        })}
      </Stack>

      <Divider />
      <Typography variant="subtitle1">Termíny</Typography>
      <LocalizationProvider dateAdapter={AdapterDateFns}>
        <Stack direction={{ xs: 'column', sm: 'row' }} spacing={2} alignItems="center">
          <TextField label="Název" value={termDraftTitle} onChange={(e) => setTermDraftTitle(e.target.value)} />
          <TextField label="Typ" value={termDraftType} onChange={(e) => setTermDraftType(e.target.value as any)} />
          <DateTimePicker label="Datum" value={termDate} onChange={(v) => setTermDate(v)} />
          <Select size="small" value={termRoom} onChange={(e) => setTermRoom(e.target.value)} displayEmpty>
            <MenuItem value=""><em>Bez místnosti</em></MenuItem>
            {db.rooms.map((r) => (<MenuItem key={r.id} value={r.id}>{r.name}</MenuItem>))}
          </Select>
          <Select size="small" value={String(termRequires)} onChange={(e) => setTermRequires(e.target.value === 'true')}>
            <MenuItem value="false">Bez registrace</MenuItem>
            <MenuItem value="true">Vyžaduje registraci</MenuItem>
          </Select>
          <Button variant="contained" onClick={() => {
            if (!termDate) return;
            db.addTerm(courseId, { title: termDraftTitle || 'Nový termín', type: termDraftType, date: termDate.toISOString(), roomId: termRoom || undefined, requiresRegistration: termRequires });
            setTermDraftTitle(''); setTermRoom(''); setTermRequires(false);
          }}>Přidat termín</Button>
        </Stack>
      </LocalizationProvider>
      <Stack spacing={1}>
        {course.terms.map((t) => (
          <Card key={t.id}><CardContent><Typography>{t.title} — {new Date(t.date).toLocaleString()}</Typography></CardContent></Card>
        ))}
      </Stack>

      <Divider />
      <Typography variant="subtitle1">Registrace studentů</Typography>
      <Stack spacing={1}>
        {pendingEnrollments.length === 0 && <Typography color="text.secondary">Žádné čekající registrace</Typography>}
        {pendingEnrollments.map((e) => {
          const u = db.users.find((x) => x.id === e.userId);
          return (
            <Stack key={e.id} direction={{ xs: 'column', sm: 'row' }} spacing={2} alignItems="center">
              <Box sx={{ flexGrow: 1 }}>
                <Typography>{u?.fullName}</Typography>
              </Box>
              <Button onClick={() => db.decideEnrollment(e.id, 'approved')} variant="contained">Schválit</Button>
              <Button onClick={() => db.decideEnrollment(e.id, 'rejected')} color="error" variant="outlined">Zamítnout</Button>
            </Stack>
          );
        })}
      </Stack>
    </Stack>
  );
}

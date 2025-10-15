import { useParams } from 'react-router-dom';
import { Box, Button, Chip, Divider, Stack, Typography } from '@mui/material';
import { format } from 'date-fns';
import { useDb } from '../../store/db';
import { useAuthStore } from '../../store/auth';

export default function CourseDetail() {
  const { id } = useParams();
  const course = useDb((s) => s.courses.find((c) => c.id === id));
  const enroll = useDb((s) => s.enroll);
  const user = useAuthStore((s) => s.user);

  const enrolled = useDb((s) => s.enrollments).find((e) => e.courseId === id && e.userId === user?.id && e.status !== 'withdrawn');

  if (!course) return <Typography>Kurz nebyl nalezen.</Typography>;

  return (
    <Stack spacing={2}>
      <Typography variant="h4">{course.title}</Typography>
      <Stack direction="row" spacing={1}>
        <Chip label={course.type} />
        <Chip label={course.status} />
      </Stack>
      {course.description && <Typography>{course.description}</Typography>}
      <Divider />
      <Typography variant="h6">Termíny</Typography>
      <Stack spacing={1}>
        {course.terms.map((t) => (
          <Box key={t.id}>
            <Typography>{t.title} — {t.type} — {format(new Date(t.date), 'd.M.yyyy HH:mm')}</Typography>
          </Box>
        ))}
      </Stack>
      <Divider />
      <Box>
        {user ? (
          enrolled ? (
            <Chip color="success" label={`Registrace: ${enrolled.status}`} />
          ) : (
            <Button variant="contained" onClick={() => user && enroll(course.id, user.id)}>Registrovat se</Button>
          )
        ) : (
          <Typography>Přihlaste se (vpravo nahoře) pro registraci.</Typography>
        )}
      </Box>
    </Stack>
  );
}

import { Box, Typography, Stack, Button } from '@mui/material';
import { Link } from 'react-router-dom';

export default function Home() {
  return (
    <Stack spacing={2}>
      <Typography variant="h4">Vítejte ve WIS2</Typography>
      <Typography variant="body1">
        Jednoduchý informační systém pro správu a registraci výukových kurzů.
      </Typography>
      <Box>
        <Button variant="contained" component={Link} to="/catalog">Procházet kurzy</Button>
      </Box>
    </Stack>
  );
}

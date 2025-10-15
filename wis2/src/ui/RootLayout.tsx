import { Suspense, useEffect, useState } from 'react';
import { Outlet, Link } from 'react-router-dom';
import { AppBar, Toolbar, Typography, Box, Container, Button, Menu, MenuItem, Tooltip, Avatar } from '@mui/material';
import { ThemeProvider } from '@mui/material/styles';
import { LocalizationProvider } from '@mui/x-date-pickers';
import { AdapterDateFns } from '@mui/x-date-pickers/AdapterDateFns';
import { theme } from '../theme';
import { useDb } from '../store/db';
import { useAuthStore } from '../store/auth';

export function RootLayout() {
  const initDb = useDb((s) => s.init);
  const users = useDb((s) => s.users);
  const setUsers = useAuthStore((s) => s.setUsers);
  const user = useAuthStore((s) => s.user);
  const logout = useAuthStore((s) => s.logout);

  useEffect(() => {
    initDb(true);
  }, [initDb]);

  useEffect(() => {
    setUsers(users);
  }, [users, setUsers]);

  return (
    <ThemeProvider theme={theme}>
      <LocalizationProvider dateAdapter={AdapterDateFns}>
      <AppBar position="sticky">
        <Toolbar sx={{ gap: 2 }}>
          <Typography variant="h6" component={Link} to="/" color="inherit" sx={{ textDecoration: 'none' }}>
            WIS2
          </Typography>
          <Button color="inherit" component={Link} to="/catalog">Catalog</Button>
          <Button color="inherit" component={Link} to="/dashboard">Dashboard</Button>
          <Button color="inherit" component={Link} to="/admin">Admin</Button>
          <Box sx={{ flexGrow: 1 }} />
          {user ? (
            <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
              <Tooltip title={`${user.fullName} (${user.role})`}>
                <Avatar sx={{ width: 32, height: 32 }}>{user.fullName[0]}</Avatar>
              </Tooltip>
              <Button color="inherit" onClick={logout}>Logout</Button>
            </Box>
          ) : (
            <UserSwitcher />
          )}
        </Toolbar>
      </AppBar>
      <Container sx={{ py: 3 }}>
        <Suspense fallback={<div>Loading...</div>}>
          <Outlet />
        </Suspense>
      </Container>
      </LocalizationProvider>
    </ThemeProvider>
  );
}

function UserSwitcher() {
  const [anchor, setAnchor] = useState<null | HTMLElement>(null);
  const users = useAuthStore((s) => s.users);
  const loginAs = useAuthStore((s) => s.loginAs);

  return (
    <>
      <Button color="inherit" onClick={(e) => setAnchor(e.currentTarget)}>Login as...</Button>
      <Menu open={Boolean(anchor)} anchorEl={anchor} onClose={() => setAnchor(null)}>
        {users.map((u) => (
          <MenuItem key={u.id} onClick={() => { loginAs(u); setAnchor(null); }}>
            {u.fullName} — {u.role}
          </MenuItem>
        ))}
      </Menu>
    </>
  );
}

export default RootLayout;

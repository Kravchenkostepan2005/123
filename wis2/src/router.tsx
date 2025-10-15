import { createBrowserRouter } from 'react-router-dom';
import { lazy } from 'react';
import { RootLayout } from './ui/RootLayout';

const Home = lazy(() => import('./ui/pages/Home'));
const Catalog = lazy(() => import('./ui/pages/Catalog'));
const CourseDetail = lazy(() => import('./ui/pages/CourseDetail'));
const Dashboard = lazy(() => import('./ui/pages/Dashboard'));
const Admin = lazy(() => import('./ui/pages/Admin'));

export const router = createBrowserRouter([
  {
    path: '/',
    element: <RootLayout />,
    children: [
      { index: true, element: <Home /> },
      { path: 'catalog', element: <Catalog /> },
      { path: 'course/:id', element: <CourseDetail /> },
      { path: 'dashboard', element: <Dashboard /> },
      { path: 'admin', element: <Admin /> },
    ],
  },
]);

import { createBrowserRouter } from 'react-router-dom';
import AppLayout from '../layouts/AppLayout';
import CatalogPage from '../pages/CatalogPage';
import CourseDetailPage from '../pages/CourseDetailPage';
import DashboardPage from '../pages/DashboardPage';
import AdminPage from '../pages/AdminPage';
import HomePage from '../pages/HomePage';

const router = createBrowserRouter([
  {
    element: <AppLayout />,
    children: [
      { index: true, element: <HomePage /> },
      { path: 'catalog', element: <CatalogPage /> },
      { path: 'catalog/:courseId', element: <CourseDetailPage /> },
      { path: 'dashboard', element: <DashboardPage /> },
      { path: 'admin', element: <AdminPage /> },
    ],
  },
]);

export default router;

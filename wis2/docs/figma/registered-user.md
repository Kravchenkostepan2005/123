WIS2 – Figma design spec: Registered user (Guarantor / Instructor / Student)

Purpose
- Define IA, screens, flows, and components in Figma for registered roles
- Align UI with functional scope: create/manage courses, terms, registrations, grading, schedules

Foundations
- Frame size: Desktop 1440×900 (auto-layout), Mobile 390×844 (optional)
- Grid: 12-col, 80px margins, 24px gutters
- Typography: Inter (or system), sizes 12/14/16/20/24/32
- Colors: Primary #2563EB, Success #16A34A, Warning #F59E0B, Danger #DC2626, Gray #111827…#F3F4F6
- Spacing scale: 4, 8, 12, 16, 20, 24, 32, 40
- Radius: 8 (inputs/cards), 12 (dialogs)
- Elevation: 1/2/4 for hover/active/modals

Information Architecture
- Global
  - Top bar: brand, search, user menu (Profile, Settings, Logout)
  - Side nav (registered): Dashboard, Courses, My Schedule, Approvals (guarantor), Grading (instructor)
- Student
  - Discover Courses (catalog), Course Detail, Register to Course, My Schedule, My Grades
- Guarantor
  - My Courses (owned), Course Editor, Terms Manager, Registrations Approvals, Instructors
- Instructor
  - Course Students list, Term List, Grade Entry, Grade Overview

Key Flows
- Student
  - Discover → Course Detail → Register → Await approval or auto-approve → See in My Schedule → Register to term (if required)
  - View Grades → per term and aggregated
- Guarantor
  - Create Course → Submit for approval → After admin approve, manage settings (type, capacity, auto-approve)
  - Manage Terms → Create/Edit/Delete; set requires_registration and capacity
  - Approve Course Registrations → approve/reject
  - Add Instructors → search user, add
- Instructor
  - View Course Students → select term → Enter/Update grades → Save

Screens (Desktop primary)
- Dashboard
  - Cards: Pending Approvals (guarantor), Upcoming Terms, Recent Grades (instructor), Enrolled Courses (student)
- Courses
  - Tabs: All (student), My Courses (guarantor/instructor)
  - Table: Code, Title, Type, Status, Capacity, Actions
- Course Detail
  - Header: Code, Title, Status, actions: Register (student), Edit (guarantor)
  - Sections: Description, News, Instructors, Terms list (date, type, registration required, capacity)
- Course Editor (Guarantor)
  - Form: Code, Title, Type, Description, Price, News, Auto-approve toggle, Capacity
  - Terms subpage: list + New Term dialog (Name, Type, Date/Time, Room, Requires registration, Capacity, Max points)
  - Instructors subpage: add/remove instructor
- Approvals (Guarantor)
  - Table of registrations: Student, Date, Status, Actions (Approve/Reject)
- Grading (Instructor)
  - Filters: Course, Term
  - Table: Student, Points, Max, Status (validated)
- My Schedule (Student)
  - Calendar list: upcoming terms for enrolled courses; CTA to register for terms
- My Grades (Student)
  - List grouped by Course → Term rows with points; total per course

Components
- App shell: TopBar, SideNav
- Inputs: TextField, Select, DateTimePicker, Toggle, NumberInput
- Tables: DataTable with sorting, pagination
- Lists: TermCard, CourseCard
- Status chips: draft/pending/approved/rejected
- Modals: New Course, New Term, Confirm Approve/Reject
- Empty states + placeholders

States & Permissions
- Student: read approved courses; register to course/term; view schedule/grades
- Guarantor: edit own courses; add instructors; approve registrations; manage terms
- Instructor: view students; set grades for terms of assigned courses (MVP owner)
- Error/empty/loading states for each screen

API Mapping (MVP)
- Auth: POST /auth/register, POST /auth/token
- Users: GET /users/me
- Courses: GET /courses, POST /courses, GET /courses/{id}, POST /courses/{id}/register, POST /courses/{id}/instructors/{user_id}
- Admin: POST /admin/courses/{id}/approve
- Terms: POST /courses/{course_id}/terms, GET /courses/{course_id}/terms
- Term registration: POST /courses/terms/{term_id}/register
- Schedule: GET /courses/schedule/me
- Students list: GET /courses/{course_id}/students
- Grades: POST /courses/terms/{term_id}/grade/{student_id}, GET /courses/terms/{term_id}/grades, GET /courses/grades/me

Acceptance Criteria
- Student can browse, register, see schedule and grades
- Guarantor can create/edit course, manage terms, approve registrations, add instructors
- Instructor can view students and assign/update grades
- UI shows loading/empty/error states; responsive layouts for 1440 and basic mobile

Figma Organization Guidance
- Pages: 00 Foundations, 10 Components, 20 Templates, 30 Screens – Student, 31 Screens – Guarantor, 32 Screens – Instructor, 90 Prototypes
- Components with variants: Button (size/state), Input (state), Select, Chip (status), Table row, Term card
- Use Auto Layout; use variables for color/spacing/typography tokens
- Link flows with prototype interactions and annotate edge cases

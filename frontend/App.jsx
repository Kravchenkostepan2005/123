import React, { useState, useEffect } from 'react';
import ActionsDemo from './actions-demo.jsx';

function TwoColumnForm({ showFormSuccess, formData, onFormChange, onFormSave, onFormReset }) {
  return (
    <div className="p-6">
      <div className="max-w-4xl mx-auto">
        {showFormSuccess && (
          <div className="mb-4 p-4 bg-green-50 border border-green-200 rounded-lg">
            <p className="text-green-700 text-sm">Form data saved successfully!</p>
          </div>
        )}

        <div className="bg-white rounded-lg shadow-sm border border-gray-200">
          <div className="px-6 py-4 border-b border-gray-200">
            <h1 className="text-lg font-semibold text-gray-900">Form title</h1>
          </div>
          <div className="p-6">
            <form onSubmit={onFormSave}>
              <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                <div className="space-y-4">
                  <div>
                    <label className="block text-sm font-medium text-gray-700 mb-2">Field A</label>
                    <input
                      type="text"
                      value={formData.fieldA}
                      onChange={(e) => onFormChange('fieldA', e.target.value)}
                      className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                      placeholder="Enter text for Field A"
                    />
                  </div>
                  <div>
                    <label className="block text-sm font-medium text-gray-700 mb-2">Field B</label>
                    <input
                      type="text"
                      value={formData.fieldB}
                      onChange={(e) => onFormChange('fieldB', e.target.value)}
                      className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                      placeholder="Enter text for Field B"
                    />
                  </div>
                </div>
                <div className="space-y-4">
                  <div>
                    <label className="block text-sm font-medium text-gray-700 mb-2">Field C</label>
                    <input
                      type="text"
                      value={formData.fieldC}
                      onChange={(e) => onFormChange('fieldC', e.target.value)}
                      className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                      placeholder="Enter text for Field C"
                    />
                  </div>
                  <div>
                    <label className="block text-sm font-medium text-gray-700 mb-2">Field D</label>
                    <input
                      type="text"
                      value={formData.fieldD}
                      onChange={(e) => onFormChange('fieldD', e.target.value)}
                      className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                      placeholder="Enter text for Field D"
                    />
                  </div>
                </div>
              </div>
              <div className="flex justify-end gap-3 mt-8 pt-6 border-t border-gray-200">
                <button
                  type="button"
                  onClick={onFormReset}
                  className="px-4 py-2 text-sm text-gray-700 bg-white border border-gray-300 rounded-md hover:bg-gray-50"
                >
                  Reset
                </button>
                <button
                  type="submit"
                  className="px-4 py-2 text-sm text-white bg-blue-600 rounded-md hover:bg-blue-700"
                >
                  Save
                </button>
              </div>
            </form>
          </div>
        </div>

        {(formData.fieldA || formData.fieldB || formData.fieldC || formData.fieldD) && (
          <div className="mt-6 bg-blue-50 border border-blue-200 rounded-lg p-4">
            <h3 className="text-sm font-medium text-blue-900 mb-2">Saved Form Data:</h3>
            <div className="text-sm text-blue-700 space-y-1">
              {formData.fieldA && <div>Field A: {formData.fieldA}</div>}
              {formData.fieldB && <div>Field B: {formData.fieldB}</div>}
              {formData.fieldC && <div>Field C: {formData.fieldC}</div>}
              {formData.fieldD && <div>Field D: {formData.fieldD}</div>}
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

export default function App() {
  const [activeTab, setActiveTab] = useState(() => {
    const saved = localStorage.getItem('wis2_activeTab');
    return saved ? saved : 'Desktop1';
  });

  const [courses, setCourses] = useState(() => {
    const saved = localStorage.getItem('wis2_courses');
    return saved
      ? JSON.parse(saved)
      : [
          { id: 1, title: 'JS101 ? JavaScript for Beginners', status: 'approved' },
          { id: 2, title: 'MATH201 ? Linear Algebra', status: 'pending' },
          { id: 3, title: 'ENG102 ? Academic English', status: 'rejected' },
        ];
  });

  const [desktopItems, setDesktopItems] = useState(() => {
    const saved = localStorage.getItem('wis2_desktopItems');
    return saved
      ? JSON.parse(saved)
      : [
          { id: 1, title: 'Project Alpha', status: 'approved' },
          { id: 2, title: 'Task Beta', status: 'pending' },
          { id: 3, title: 'Document Gamma', status: 'rejected' },
        ];
  });

  const [dataTableItems, setDataTableItems] = useState(() => {
    const saved = localStorage.getItem('wis2_dataTableItems');
    return saved
      ? JSON.parse(saved)
      : [
          { id: 1, columnA: 'Data A1', columnB: 'Data B1', columnC: 'Data C1', status: 'Active' },
          { id: 2, columnA: 'Data A2', columnB: 'Data B2', columnC: 'Data C2', status: 'Pending' },
          { id: 3, columnA: 'Data A3', columnB: 'Data B3', columnC: 'Data C3', status: 'Inactive' },
        ];
  });

  const [formData, setFormData] = useState(() => {
    const saved = localStorage.getItem('wis2_formData');
    return saved
      ? JSON.parse(saved)
      : {
          fieldA: '',
          fieldB: '',
          fieldC: '',
          fieldD: '',
        };
  });

  const [showFormSuccess, setShowFormSuccess] = useState(false);
  const [guarantorSection, setGuarantorSection] = useState('dashboard');

  const [courseEditorData, setCourseEditorData] = useState({
    name: 'Internet',
    owner: '',
    address: '',
    agency: '',
    date: '',
    location: '',
    country: '',
    sourceEditor: '',
    code: '',
    executor: '',
    execDate: '',
    execLocation: '',
    execCountry: '',
  });

  const [instructorsData, setInstructorsData] = useState([
    { id: 1, type: 'Instruct', status: 'active' },
    { id: 2, type: 'Training', status: 'active' },
    { id: 3, type: 'Writing', status: 'inactive' },
    { id: 4, type: 'Speech', status: 'active' },
    { id: 5, type: 'Dating', status: 'inactive' },
  ]);

  const [termsData, setTermsData] = useState([
    { id: 1, name: 'Academic Policy', version: '1.0', status: 'active' },
    { id: 2, name: 'Code of Conduct', version: '2.1', status: 'draft' },
    { id: 3, name: 'Privacy Policy', version: '1.5', status: 'active' },
  ]);

  const [pendingTerms, setPendingTerms] = useState([]);

  const [guarantorProfile, setGuarantorProfile] = useState(() => {
    const saved = localStorage.getItem('wis2_guarantorProfile');
    return saved
      ? JSON.parse(saved)
      : {
          name: 'Taylor Guarantor',
          email: 'taylor.guarantor@example.edu',
          department: 'Academic Affairs',
          phone: '+1 (555) 123-4567',
          bio: 'Primary guarantor for accreditation and compliance activities.',
        };
  });

  const [showProfileSuccess, setShowProfileSuccess] = useState(false);

  const [newTermForm, setNewTermForm] = useState({
    name: '',
    version: '1.0',
    description: '',
  });

  const [showNewTermSuccess, setShowNewTermSuccess] = useState(false);

  useEffect(() => {
    localStorage.setItem('wis2_activeTab', activeTab);
  }, [activeTab]);

  useEffect(() => {
    localStorage.setItem('wis2_courses', JSON.stringify(courses));
  }, [courses]);

  useEffect(() => {
    localStorage.setItem('wis2_desktopItems', JSON.stringify(desktopItems));
  }, [desktopItems]);

  useEffect(() => {
    localStorage.setItem('wis2_dataTableItems', JSON.stringify(dataTableItems));
  }, [dataTableItems]);

  useEffect(() => {
    localStorage.setItem('wis2_formData', JSON.stringify(formData));
  }, [formData]);

  useEffect(() => {
    localStorage.setItem('wis2_guarantorProfile', JSON.stringify(guarantorProfile));
  }, [guarantorProfile]);

  useEffect(() => {
    if (showFormSuccess) {
      const timer = setTimeout(() => setShowFormSuccess(false), 3000);
      return () => clearTimeout(timer);
    }
  }, [showFormSuccess]);

  useEffect(() => {
    if (showProfileSuccess) {
      const timer = setTimeout(() => setShowProfileSuccess(false), 3000);
      return () => clearTimeout(timer);
    }
  }, [showProfileSuccess]);

  useEffect(() => {
    if (showNewTermSuccess) {
      const timer = setTimeout(() => setShowNewTermSuccess(false), 3000);
      return () => clearTimeout(timer);
    }
  }, [showNewTermSuccess]);

  const handleCreateCourse = () => {
    const newCourse = { id: Date.now(), title: `New course #${courses.length + 1}`, status: 'pending' };
    setCourses((prev) => [newCourse, ...prev]);
  };

  const handleApprovePending = () => {
    setCourses((prev) => prev.map((c) => (c.status === 'pending' ? { ...c, status: 'approved' } : c)));
  };

  const handleRejectPending = () => {
    setCourses((prev) => prev.map((c) => (c.status === 'pending' ? { ...c, status: 'rejected' } : c)));
  };

  const handleApproveOne = (id) => {
    setCourses((prev) => prev.map((c) => (c.id === id ? { ...c, status: 'approved' } : c)));
  };

  const handleRejectOne = (id) => {
    setCourses((prev) => prev.map((c) => (c.id === id ? { ...c, status: 'rejected' } : c)));
  };

  const handleAddItem = () => {
    const newItem = { id: Date.now(), title: `New Item #${desktopItems.length + 1}`, status: 'pending' };
    setDesktopItems((prev) => [newItem, ...prev]);
  };

  const handleApproveAllPending = () => {
    setDesktopItems((prev) => prev.map((item) => (item.status === 'pending' ? { ...item, status: 'approved' } : item)));
  };

  const handleRejectAllPending = () => {
    setDesktopItems((prev) => prev.map((item) => (item.status === 'pending' ? { ...item, status: 'rejected' } : item)));
  };

  const handleApproveItem = (id) => {
    setDesktopItems((prev) => prev.map((item) => (item.id === id ? { ...item, status: 'approved' } : item)));
  };

  const handleRejectItem = (id) => {
    setDesktopItems((prev) => prev.map((item) => (item.id === id ? { ...item, status: 'rejected' } : item)));
  };

  const handleAddDataTableItem = () => {
    const newItem = {
      id: Date.now(),
      columnA: `Data A${dataTableItems.length + 1}`,
      columnB: `Data B${dataTableItems.length + 1}`,
      columnC: `Data C${dataTableItems.length + 1}`,
      status: 'Pending',
    };
    setDataTableItems((prev) => [...prev, newItem]);
  };

  const handleDeleteDataTableItem = (id) => {
    setDataTableItems((prev) => prev.filter((item) => item.id !== id));
  };

  const handleUpdateDataTableStatus = (id, newStatus) => {
    setDataTableItems((prev) => prev.map((item) => (item.id === id ? { ...item, status: newStatus } : item)));
  };

  const handleFormChange = (field, value) => {
    setFormData((prev) => ({ ...prev, [field]: value }));
  };

  const handleFormSave = (e) => {
    e.preventDefault();
    setShowFormSuccess(true);
  };

  const handleFormReset = () => {
    setFormData({ fieldA: '', fieldB: '', fieldC: '', fieldD: '' });
  };

  const handleCourseEditorChange = (field, value) => {
    setCourseEditorData((prev) => ({ ...prev, [field]: value }));
  };

  const handleCourseEditorSave = () => {
    console.log('Course data saved:', courseEditorData);
  };

  const toggleInstructorStatus = (id) => {
    setInstructorsData((prev) =>
      prev.map((instructor) =>
        instructor.id === id
          ? { ...instructor, status: instructor.status === 'active' ? 'inactive' : 'active' }
          : instructor
      )
    );
  };

  const handleTermStatusChange = (id, newStatus) => {
    setTermsData((prev) => prev.map((term) => (term.id === id ? { ...term, status: newStatus } : term)));
  };

  const handleDeleteTerm = (id) => {
    setTermsData((prev) => prev.filter((term) => term.id !== id));
  };

  const handleAddNewTerm = (termOrEvent) => {
    const payload = termOrEvent && termOrEvent.preventDefault ? undefined : termOrEvent;
    const newTerm = {
      id: Date.now(),
      name: payload?.name?.trim() || `New Term ${pendingTerms.length + 1}`,
      version: payload?.version?.trim() || '1.0',
      status: 'pending_approval',
      createdAt: new Date().toISOString(),
      description: payload?.description?.trim() || '',
    };

    setPendingTerms((prev) => [...prev, newTerm]);
  };

  const handleApproveTerm = (id) => {
    const termToApprove = pendingTerms.find((term) => term.id === id);
    if (termToApprove) {
      const approvedTerm = {
        ...termToApprove,
        status: 'draft',
      };
      setTermsData((prev) => [...prev, approvedTerm]);
      setPendingTerms((prev) => prev.filter((term) => term.id !== id));
    }
  };

  const handleRejectTerm = (id) => {
    setPendingTerms((prev) => prev.filter((term) => term.id !== id));
  };

  const handleProfileChange = (field, value) => {
    setGuarantorProfile((prev) => ({ ...prev, [field]: value }));
  };

  const handleProfileReset = () => {
    setGuarantorProfile({
      name: 'Taylor Guarantor',
      email: 'taylor.guarantor@example.edu',
      department: 'Academic Affairs',
      phone: '+1 (555) 123-4567',
      bio: 'Primary guarantor for accreditation and compliance activities.',
    });
  };

  const handleProfileSave = (event) => {
    if (event) {
      event.preventDefault();
    }
    setShowProfileSuccess(true);
  };

  const handleNewTermFormChange = (field, value) => {
    setNewTermForm((prev) => ({ ...prev, [field]: value }));
  };

  const handleSubmitNewTerm = (event) => {
    event.preventDefault();
    if (!newTermForm.name.trim()) {
      return;
    }

    handleAddNewTerm(newTermForm);
    setNewTermForm({ name: '', version: '1.0', description: '' });
    setShowNewTermSuccess(true);
  };

  const getStatusColor = (status) => {
    switch (status) {
      case 'approved':
        return 'bg-emerald-50 text-emerald-700 ring-1 ring-emerald-200';
      case 'pending':
        return 'bg-amber-50 text-amber-700 ring-1 ring-amber-200';
      case 'rejected':
        return 'bg-rose-50 text-rose-700 ring-1 ring-rose-200';
      case 'pending_approval':
        return 'bg-blue-50 text-blue-700 ring-1 ring-blue-200';
      default:
        return 'bg-gray-50 text-gray-700 ring-1 ring-gray-200';
    }
  };

  const getTableStatusColor = (status) => {
    switch (status) {
      case 'Active':
        return 'bg-green-50 text-green-700';
      case 'Pending':
        return 'bg-yellow-50 text-yellow-700';
      case 'Inactive':
        return 'bg-red-50 text-red-700';
      default:
        return 'bg-gray-50 text-gray-700';
    }
  };

  const Courses = () => (
    <div className="p-6">
      <h1 className="text-2xl font-bold text-gray-900 mb-6">Courses</h1>
      <div className="bg-white rounded-lg border border-gray-200 overflow-hidden">
        <table className="w-full">
          <thead className="bg-gray-50">
            <tr>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Course</th>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Status</th>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Actions</th>
            </tr>
          </thead>
          <tbody>
            {courses.map((course) => (
              <tr key={course.id} className="border-b border-gray-200 hover:bg-gray-50">
                <td className="px-6 py-4">
                  <div className="font-medium text-gray-900">{course.title}</div>
                </td>
                <td className="px-6 py-4">
                  <span
                    className={`inline-flex items-center rounded-full px-2.5 py-0.5 text-xs font-medium ${getStatusColor(
                      course.status
                    )}`}
                  >
                    {course.status}
                  </span>
                </td>
                <td className="px-6 py-4">
                  <div className="flex gap-2">
                    <button
                      onClick={() => handleApproveOne(course.id)}
                      className="inline-flex items-center gap-1 rounded border border-green-600 text-green-700 px-2 py-1 hover:bg-green-50 text-xs"
                    >
                      ? Approve
                    </button>
                    <button
                      onClick={() => handleRejectOne(course.id)}
                      className="inline-flex items-center gap-1 rounded border border-red-600 text-red-700 px-2 py-1 hover:bg-red-50 text-xs"
                    >
                      ? Reject
                    </button>
                  </div>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );

  const Desktop1 = () => (
    <div className="p-6">
      <h1 className="text-2xl font-bold text-gray-900 mb-8">Desktop - 1</h1>
      <h2 className="text-xl font-semibold text-gray-800 mb-4">WG32</h2>

      <div className="space-y-6">
        <div>
          <h3 className="text-lg font-medium text-gray-700 mb-3">Actions demo</h3>
          <div className="flex gap-3 mb-4">
            <button
              onClick={handleAddItem}
              className="bg-blue-600 hover:bg-blue-700 text-white px-4 py-2 rounded-md text-sm"
            >
              Add
            </button>
            <button
              onClick={handleApproveAllPending}
              className="bg-green-600 hover:bg-green-700 text-white px-4 py-2 rounded-md text-sm"
            >
              Approve
            </button>
            <button
              onClick={handleRejectAllPending}
              className="bg-red-600 hover:bg-red-700 text-white px-4 py-2 rounded-md text-sm"
            >
              Cancel
            </button>
          </div>

          <div className="border border-gray-200 rounded-md overflow-hidden">
            <table className="w-full text-sm">
              <thead className="bg-gray-50">
                <tr>
                  <th className="text-left px-4 py-2 border-b">Item</th>
                  <th className="text-left px-4 py-2 border-b">Status</th>
                  <th className="text-left px-4 py-2 border-b">Actions</th>
                </tr>
              </thead>
              <tbody>
                {desktopItems
                  .filter((item) => item.status === 'pending')
                  .map((item) => (
                    <tr key={item.id} className="border-b hover:bg-gray-50">
                      <td className="px-4 py-2">{item.title}</td>
                      <td className="px-4 py-2">
                        <span
                          className={`inline-flex items-center rounded-full px-2 py-0.5 text-xs ${getStatusColor(
                            item.status
                          )}`}
                        >
                          {item.status}
                        </span>
                      </td>
                      <td className="px-4 py-2">
                        <div className="flex gap-2">
                          <button
                            onClick={() => handleApproveItem(item.id)}
                            className="inline-flex items-center gap-1 rounded border border-green-600 text-green-700 px-2 py-1 hover:bg-green-50 text-xs"
                          >
                            ? Approve
                          </button>
                          <button
                            onClick={() => handleRejectItem(item.id)}
                            className="inline-flex items-center gap-1 rounded border border-red-600 text-red-700 px-2 py-1 hover:bg-red-50 text-xs"
                          >
                            ? Reject
                          </button>
                        </div>
                      </td>
                    </tr>
                  ))}
                {desktopItems.filter((item) => item.status === 'pending').length === 0 && (
                  <tr>
                    <td colSpan={3} className="px-4 py-6 text-center text-gray-500">
                      No pending items
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
        </div>

        <div>
          <h3 className="text-lg font-medium text-gray-700 mb-3">Item</h3>
          <div className="bg-white p-4 rounded-md border border-gray-200">
            <span className="text-gray-600">
              Status: {desktopItems.filter((item) => item.status === 'pending').length} pending
            </span>
          </div>
        </div>
      </div>
    </div>
  );

  const AppShell = () => (
    <div className="p-6">
      <div className="grid grid-cols-12 gap-4">
        <div className="col-span-12">
          <h1 className="text-lg font-semibold text-gray-900">Dashboard</h1>
        </div>

        <div className="col-span-12 md:col-span-6 lg:col-span-3 bg-white p-4 rounded-lg border border-gray-200">
          <h3 className="font-medium text-gray-700">Contrast</h3>
        </div>

        <div className="col-span-12 md:col-span-6 lg:col-span-3 bg-white p-4 rounded-lg border border-gray-200">
          <h3 className="font-medium text-gray-700">My Schedule</h3>
        </div>

        <div className="col-span-12 md:col-span-6 lg:col-span-3 bg-white p-4 rounded-lg border border-gray-200">
          <h3 className="font-medium text-gray-700">Appendix</h3>
        </div>

        <div className="col-span-12 md:col-span-6 lg:col-span-3 bg-white p-4 rounded-lg border border-gray-200">
          <h3 className="font-medium text-gray-700">Ordering</h3>
        </div>

        <div className="col-span-12 bg-white p-6 rounded-lg border border-gray-200">
          <h3 className="font-medium text-gray-700 mb-4">Content area (12-col grid layout)</h3>
          <div className="grid grid-cols-12 gap-4">
            {Array.from({ length: 12 }).map((_, i) => (
              <div key={i} className="col-span-12 sm:col-span-6 md:col-span-4 lg:col-span-3 xl:col-span-2 h-8 bg-gray-100 rounded" />
            ))}
          </div>
        </div>
      </div>
    </div>
  );

  const DataTable = () => (
    <div className="p-6">
      <div className="max-w-6xl mx-auto">
        <div className="mb-6">
          <h1 className="text-2xl font-bold text-gray-900">data-table 1</h1>
        </div>
        <div className="bg-white rounded-lg border border-gray-200 p-4 mb-6">
          <div className="flex flex-col sm:flex-row gap-4 justify-between items-start sm:items-center">
            <div className="text-lg font-semibold text-gray-900">Page title</div>
            <div className="flex gap-4">
              <input
                type="text"
                placeholder="Search..."
                className="pl-3 pr-4 py-2 border border-gray-300 rounded-md text-sm"
              />
              <button className="px-4 py-2 text-sm text-gray-700 bg-white border border-gray-300 rounded-md">
                Filter
              </button>
              <button
                onClick={handleAddDataTableItem}
                className="px-4 py-2 text-sm text-white bg-blue-600 rounded-md"
              >
                Add Row
              </button>
            </div>
          </div>
        </div>
        <div className="bg-white rounded-lg border border-gray-200 overflow-hidden">
          <table className="w-full">
            <thead className="bg-gray-50">
              <tr>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Column A</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Column B</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Column C</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Status</th>
                <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Actions</th>
              </tr>
            </thead>
            <tbody>
              {dataTableItems.map((item) => (
                <tr key={item.id} className="border-b border-gray-200">
                  <td className="px-6 py-4 text-sm text-gray-900">{item.columnA}</td>
                  <td className="px-6 py-4 text-sm text-gray-900">{item.columnB}</td>
                  <td className="px-6 py-4 text-sm text-gray-900">{item.columnC}</td>
                  <td className="px-6 py-4">
                    <span
                      className={`inline-flex items-center rounded-full px-2 py-1 text-xs ${getTableStatusColor(
                        item.status
                      )}`}
                    >
                      {item.status}
                    </span>
                  </td>
                  <td className="px-6 py-4">
                    <div className="flex gap-2">
                      <button
                        onClick={() => handleUpdateDataTableStatus(item.id, 'Active')}
                        className="text-xs text-green-600 hover:text-green-800"
                      >
                        Active
                      </button>
                      <button
                        onClick={() => handleUpdateDataTableStatus(item.id, 'Pending')}
                        className="text-xs text-yellow-600 hover:text-yellow-800"
                      >
                        Pending
                      </button>
                      <button
                        onClick={() => handleUpdateDataTableStatus(item.id, 'Inactive')}
                        className="text-xs text-red-600 hover:text-red-800"
                      >
                        Inactive
                      </button>
                      <button
                        onClick={() => handleDeleteDataTableItem(item.id)}
                        className="text-xs text-gray-600 hover:text-gray-800"
                      >
                        Delete
                      </button>
                    </div>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );

  const Guarantor = () => {
    const GuarantorNavigation = () => (
      <div className="bg-white rounded-lg border border-gray-200 p-4 mb-6">
        <div className="flex gap-2 flex-wrap">
          <button
            onClick={() => setGuarantorSection('dashboard')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'dashboard'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            Dashboard
          </button>
          <button
            onClick={() => setGuarantorSection('approvals')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'approvals'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            Approvals
          </button>
          <button
            onClick={() => setGuarantorSection('myCourses')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'myCourses'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            My Courses
          </button>
          <button
            onClick={() => setGuarantorSection('courseEditor')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'courseEditor'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            Course Editor
          </button>
          <button
            onClick={() => setGuarantorSection('instructors')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'instructors'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            Instructors
          </button>
          <button
            onClick={() => setGuarantorSection('termsManager')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'termsManager'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            Terms Manager
          </button>
          <button
            onClick={() => setGuarantorSection('adminApprovals')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'adminApprovals'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            Admin Approvals ({pendingTerms.length})
          </button>
          <button
            onClick={() => setGuarantorSection('editProfile')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'editProfile'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            Edit Profile
          </button>
          <button
            onClick={() => setGuarantorSection('newTermDialog')}
            className={`px-4 py-2 rounded-lg transition-colors text-sm ${
              guarantorSection === 'newTermDialog'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
            }`}
          >
            New Term Dialog
          </button>
        </div>
      </div>
    );

    const MyCoursesSection = () => {
      const statusSummary = courses.reduce(
        (acc, course) => {
          const key = course.status;
          acc[key] = (acc[key] || 0) + 1;
          return acc;
        },
        { approved: 0, pending: 0, rejected: 0 }
      );

      return (
        <div className="space-y-6">
          <h2 className="text-xl font-semibold text-gray-900">My Courses</h2>

          <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
            <div className="bg-white border border-gray-200 rounded-lg p-4">
              <p className="text-sm text-gray-500">Approved</p>
              <p className="text-2xl font-bold text-emerald-600">{statusSummary.approved}</p>
            </div>
            <div className="bg-white border border-gray-200 rounded-lg p-4">
              <p className="text-sm text-gray-500">Pending</p>
              <p className="text-2xl font-bold text-amber-600">{statusSummary.pending}</p>
            </div>
            <div className="bg-white border border-gray-200 rounded-lg p-4">
              <p className="text-sm text-gray-500">Rejected</p>
              <p className="text-2xl font-bold text-rose-600">{statusSummary.rejected}</p>
            </div>
          </div>

          <div className="bg-white rounded-lg border border-gray-200 overflow-hidden">
            <table className="w-full text-sm">
              <thead className="bg-gray-50">
                <tr>
                  <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Course</th>
                  <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Status</th>
                  <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Last Updated</th>
                </tr>
              </thead>
              <tbody>
                {courses.map((course) => (
                  <tr key={course.id} className="border-b border-gray-200 hover:bg-gray-50">
                    <td className="px-6 py-4 font-medium text-gray-900">{course.title}</td>
                    <td className="px-6 py-4">
                      <span
                        className={`inline-flex items-center rounded-full px-2.5 py-0.5 text-xs font-medium ${getStatusColor(
                          course.status
                        )}`}
                      >
                        {course.status}
                      </span>
                    </td>
                    <td className="px-6 py-4 text-gray-500">Recent</td>
                  </tr>
                ))}
                {courses.length === 0 && (
                  <tr>
                    <td colSpan={3} className="px-6 py-12 text-center text-gray-500">
                      No courses assigned yet
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
        </div>
      );
    };

    const CourseEditorSection = () => (
      <div className="space-y-6">
        <h2 className="text-xl font-semibold text-gray-900">Course Editor</h2>

        <div className="bg-white rounded-lg border border-gray-200 p-6">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Name</label>
                <input
                  type="text"
                  value={courseEditorData.name}
                  onChange={(e) => handleCourseEditorChange('name', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Owner</label>
                <input
                  type="text"
                  value={courseEditorData.owner}
                  onChange={(e) => handleCourseEditorChange('owner', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Address</label>
                <input
                  type="text"
                  value={courseEditorData.address}
                  onChange={(e) => handleCourseEditorChange('address', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Agency</label>
                <input
                  type="text"
                  value={courseEditorData.agency}
                  onChange={(e) => handleCourseEditorChange('agency', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Date</label>
                <input
                  type="date"
                  value={courseEditorData.date}
                  onChange={(e) => handleCourseEditorChange('date', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Location</label>
                <input
                  type="text"
                  value={courseEditorData.location}
                  onChange={(e) => handleCourseEditorChange('location', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Country</label>
                <input
                  type="text"
                  value={courseEditorData.country}
                  onChange={(e) => handleCourseEditorChange('country', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
            </div>

            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Source Editor</label>
                <input
                  type="text"
                  value={courseEditorData.sourceEditor}
                  onChange={(e) => handleCourseEditorChange('sourceEditor', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Code</label>
                <input
                  type="text"
                  value={courseEditorData.code}
                  onChange={(e) => handleCourseEditorChange('code', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Executor</label>
                <input
                  type="text"
                  value={courseEditorData.executor}
                  onChange={(e) => handleCourseEditorChange('executor', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Date</label>
                <input
                  type="date"
                  value={courseEditorData.execDate}
                  onChange={(e) => handleCourseEditorChange('execDate', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Location</label>
                <input
                  type="text"
                  value={courseEditorData.execLocation}
                  onChange={(e) => handleCourseEditorChange('execLocation', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2">Country</label>
                <input
                  type="text"
                  value={courseEditorData.execCountry}
                  onChange={(e) => handleCourseEditorChange('execCountry', e.target.value)}
                  className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                />
              </div>
            </div>
          </div>

          <div className="flex justify-end gap-3 mt-8 pt-6 border-t border-gray-200">
            <button className="px-4 py-2 text-sm text-gray-700 bg-white border border-gray-300 rounded-md hover:bg-gray-50">
              Reset
            </button>
            <button
              onClick={handleCourseEditorSave}
              className="px-4 py-2 text-sm text-white bg-blue-600 rounded-md hover:bg-blue-700"
            >
              Save
            </button>
          </div>
        </div>
      </div>
    );

    const InstructorsSection = () => (
      <div className="space-y-6">
        <h2 className="text-xl font-semibold text-gray-900">Instructors</h2>

        <div className="bg-white rounded-lg border border-gray-200 p-6">
          <h3 className="text-lg font-medium text-gray-700 mb-4">Code</h3>
          <div className="space-y-2">
            {instructorsData.map((instructor) => (
              <div
                key={instructor.id}
                className="flex items-center justify-between p-3 border border-gray-200 rounded-lg"
              >
                <span className="text-gray-700">{instructor.type}</span>
                <button
                  onClick={() => toggleInstructorStatus(instructor.id)}
                  className={`px-3 py-1 rounded text-sm ${
                    instructor.status === 'active'
                      ? 'bg-green-100 text-green-800'
                      : 'bg-gray-100 text-gray-800'
                  }`}
                >
                  {instructor.status === 'active' ? 'Active' : 'Inactive'}
                </button>
              </div>
            ))}
          </div>
        </div>
      </div>
    );

    const TermsManagerSection = () => (
      <div className="space-y-6">
        <h2 className="text-xl font-semibold text-gray-900">Terms Manager</h2>

        <div className="bg-white rounded-lg border border-gray-200 p-6">
          <div className="flex justify-between items-center mb-6">
            <h3 className="text-lg font-medium text-gray-700">Manage Terms & Policies</h3>
            <button
              onClick={() => handleAddNewTerm()}
              className="px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 text-sm"
            >
              Add New Term
            </button>
          </div>

          <div className="space-y-4">
            {termsData.map((term) => (
              <div
                key={term.id}
                className="flex items-center justify-between p-4 border border-gray-200 rounded-lg"
              >
                <div>
                  <h4 className="font-medium text-gray-900">{term.name}</h4>
                  <p className="text-sm text-gray-500">Version {term.version}</p>
                </div>
                <div className="flex items-center gap-3">
                  <span
                    className={`inline-flex items-center rounded-full px-2.5 py-0.5 text-xs font-medium ${
                      term.status === 'active'
                        ? 'bg-green-100 text-green-800'
                        : term.status === 'draft'
                        ? 'bg-yellow-100 text-yellow-800'
                        : 'bg-gray-100 text-gray-800'
                    }`}
                  >
                    {term.status}
                  </span>
                  <div className="flex gap-2">
                    <button
                      onClick={() => handleTermStatusChange(term.id, 'active')}
                      className="text-green-600 hover:text-green-800 text-sm"
                    >
                      Activate
                    </button>
                    <button
                      onClick={() => handleTermStatusChange(term.id, 'draft')}
                      className="text-yellow-600 hover:text-yellow-800 text-sm"
                    >
                      Draft
                    </button>
                    <button
                      onClick={() => handleDeleteTerm(term.id)}
                      className="text-red-600 hover:text-red-800 text-sm"
                    >
                      Delete
                    </button>
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    );

    const AdminApprovalsSection = () => (
      <div className="space-y-6">
        <h2 className="text-xl font-semibold text-gray-900">Admin Approvals</h2>
        <p className="text-gray-600">Pending terms awaiting approval</p>

        <div className="bg-white rounded-lg border border-gray-200 p-6">
          {pendingTerms.length === 0 ? (
            <div className="text-center py-8 text-gray-500">No pending terms for approval</div>
          ) : (
            <div className="space-y-4">
              {pendingTerms.map((term) => (
                <div
                  key={term.id}
                  className="flex items-center justify-between p-4 border border-gray-200 rounded-lg"
                >
                  <div>
                    <h4 className="font-medium text-gray-900">{term.name}</h4>
                    <p className="text-sm text-gray-500">Version {term.version}</p>
                    {term.description && (
                      <p className="text-xs text-gray-400 mt-1">{term.description}</p>
                    )}
                    <p className="text-xs text-gray-400">
                      Created: {new Date(term.createdAt).toLocaleDateString()}
                    </p>
                  </div>
                  <div className="flex items-center gap-3">
                    <span
                      className={`inline-flex items-center rounded-full px-2.5 py-0.5 text-xs font-medium ${getStatusColor(
                        term.status
                      )}`}
                    >
                      Awaiting Approval
                    </span>
                    <div className="flex gap-2">
                      <button
                        onClick={() => handleApproveTerm(term.id)}
                        className="inline-flex items-center gap-1 rounded border border-green-600 text-green-700 px-2 py-1 hover:bg-green-50 text-xs"
                      >
                        ? Approve
                      </button>
                      <button
                        onClick={() => handleRejectTerm(term.id)}
                        className="inline-flex items-center gap-1 rounded border border-red-600 text-red-700 px-2 py-1 hover:bg-red-50 text-xs"
                      >
                        ? Reject
                      </button>
                    </div>
                  </div>
                </div>
              ))}
            </div>
          )}
        </div>
      </div>
    );

    const EditProfileSection = () => (
      <div className="space-y-6">
        <h2 className="text-xl font-semibold text-gray-900">Edit Profile</h2>

        {showProfileSuccess && (
          <div className="p-4 bg-green-50 border border-green-200 text-green-700 rounded-lg text-sm">
            Profile updated successfully.
          </div>
        )}

        <form
          onSubmit={handleProfileSave}
          className="bg-white rounded-lg border border-gray-200 p-6 space-y-4"
        >
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">Name</label>
              <input
                type="text"
                value={guarantorProfile.name}
                onChange={(e) => handleProfileChange('name', e.target.value)}
                className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">Email</label>
              <input
                type="email"
                value={guarantorProfile.email}
                onChange={(e) => handleProfileChange('email', e.target.value)}
                className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">Department</label>
              <input
                type="text"
                value={guarantorProfile.department}
                onChange={(e) => handleProfileChange('department', e.target.value)}
                className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">Phone</label>
              <input
                type="text"
                value={guarantorProfile.phone}
                onChange={(e) => handleProfileChange('phone', e.target.value)}
                className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
            </div>
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">Short Bio</label>
            <textarea
              value={guarantorProfile.bio}
              onChange={(e) => handleProfileChange('bio', e.target.value)}
              rows={4}
              className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
          </div>

          <div className="flex justify-end gap-3 pt-4 border-t border-gray-200">
            <button
              type="button"
              onClick={handleProfileReset}
              className="px-4 py-2 text-sm text-gray-700 bg-white border border-gray-300 rounded-md hover:bg-gray-50"
            >
              Reset
            </button>
            <button
              type="submit"
              className="px-4 py-2 text-sm text-white bg-blue-600 rounded-md hover:bg-blue-700"
            >
              Save Changes
            </button>
          </div>
        </form>
      </div>
    );

    const NewTermDialogSection = () => (
      <div className="space-y-6">
        <h2 className="text-xl font-semibold text-gray-900">New Term Dialog</h2>
        <p className="text-gray-600">
          Capture details for a new policy or term and send it to the admin approval queue.
        </p>

        {showNewTermSuccess && (
          <div className="p-4 bg-blue-50 border border-blue-200 text-blue-700 rounded-lg text-sm">
            Term submitted for admin approval.
          </div>
        )}

        <form
          onSubmit={handleSubmitNewTerm}
          className="bg-white rounded-lg border border-gray-200 p-6 space-y-4"
        >
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">Term Name</label>
              <input
                type="text"
                value={newTermForm.name}
                onChange={(e) => handleNewTermFormChange('name', e.target.value)}
                className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
                placeholder="e.g. Student Code Update"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">Version</label>
              <input
                type="text"
                value={newTermForm.version}
                onChange={(e) => handleNewTermFormChange('version', e.target.value)}
                className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              />
            </div>
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">Description</label>
            <textarea
              rows={4}
              value={newTermForm.description}
              onChange={(e) => handleNewTermFormChange('description', e.target.value)}
              className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              placeholder="Provide a short summary of the policy change"
            />
          </div>

          <div className="flex justify-end gap-3 pt-4 border-t border-gray-200">
            <button
              type="button"
              onClick={() => setNewTermForm({ name: '', version: '1.0', description: '' })}
              className="px-4 py-2 text-sm text-gray-700 bg-white border border-gray-300 rounded-md hover:bg-gray-50"
            >
              Clear
            </button>
            <button
              type="submit"
              className="px-4 py-2 text-sm text-white bg-blue-600 rounded-md hover:bg-blue-700"
            >
              Submit for Approval
            </button>
          </div>
        </form>

        {pendingTerms.length > 0 && (
          <div className="bg-white rounded-lg border border-gray-200 p-6">
            <h3 className="text-lg font-medium text-gray-900 mb-4">Recently Submitted</h3>
            <ul className="space-y-3 text-sm text-gray-600">
              {pendingTerms.slice(0, 3).map((term) => (
                <li key={term.id} className="flex items-start justify-between">
                  <div>
                    <p className="font-medium text-gray-900">{term.name}</p>
                    <p className="text-xs text-gray-500">Version {term.version}</p>
                  </div>
                  <span className="text-xs px-2 py-1 rounded bg-blue-50 text-blue-700">
                    Pending
                  </span>
                </li>
              ))}
            </ul>
          </div>
        )}
      </div>
    );

    return (
      <div className="p-6">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Guarantor Dashboard</h1>

        <GuarantorNavigation />

        {guarantorSection === 'dashboard' && (
          <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
            <div className="bg-white p-6 rounded-lg border border-gray-200">
              <h3 className="font-semibold text-gray-900 mb-2">Total Courses</h3>
              <p className="text-2xl font-bold text-blue-600">{courses.length}</p>
            </div>
            <div className="bg-white p-6 rounded-lg border border-gray-200">
              <h3 className="font-semibold text-gray-900 mb-2">Pending Approvals</h3>
              <p className="text-2xl font-bold text-amber-600">
                {desktopItems.filter((item) => item.status === 'pending').length}
              </p>
            </div>
            <div className="bg-white p-6 rounded-lg border border-gray-200">
              <h3 className="font-semibold text-gray-900 mb-2">Approved Items</h3>
              <p className="text-2xl font-bold text-green-600">
                {desktopItems.filter((item) => item.status === 'approved').length}
              </p>
            </div>
          </div>
        )}

        {guarantorSection === 'approvals' && (
          <div>
            <h2 className="text-xl font-semibold text-gray-900 mb-4">Pending Approvals</h2>
            <div className="bg-white rounded-lg border border-gray-200 overflow-hidden">
              <table className="w-full">
                <thead className="bg-gray-50">
                  <tr>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Item</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Status</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Actions</th>
                  </tr>
                </thead>
                <tbody>
                  {desktopItems
                    .filter((item) => item.status === 'pending')
                    .map((item) => (
                      <tr key={item.id} className="border-b border-gray-200 hover:bg-gray-50">
                        <td className="px-6 py-4">
                          <div className="font-medium text-gray-900">{item.title}</div>
                        </td>
                        <td className="px-6 py-4">
                          <span
                            className={`inline-flex items-center rounded-full px-2.5 py-0.5 text-xs font-medium ${getStatusColor(
                              item.status
                            )}`}
                          >
                            {item.status}
                          </span>
                        </td>
                        <td className="px-6 py-4">
                          <div className="flex gap-2">
                            <button
                              onClick={() => handleApproveItem(item.id)}
                              className="inline-flex items-center gap-1 rounded border border-green-600 text-green-700 px-2 py-1 hover:bg-green-50 text-xs"
                            >
                              ? Approve
                            </button>
                            <button
                              onClick={() => handleRejectItem(item.id)}
                              className="inline-flex items-center gap-1 rounded border border-red-600 text-red-700 px-2 py-1 hover:bg-red-50 text-xs"
                            >
                              ? Reject
                            </button>
                          </div>
                        </td>
                      </tr>
                    ))}
                  {desktopItems.filter((item) => item.status === 'pending').length === 0 && (
                    <tr>
                      <td colSpan="3" className="px-6 py-8 text-center text-gray-500">
                        No pending approval requests
                      </td>
                    </tr>
                  )}
                </tbody>
              </table>
            </div>
          </div>
        )}

        {guarantorSection === 'myCourses' && <MyCoursesSection />}
        {guarantorSection === 'courseEditor' && <CourseEditorSection />}
        {guarantorSection === 'instructors' && <InstructorsSection />}
        {guarantorSection === 'termsManager' && <TermsManagerSection />}
        {guarantorSection === 'adminApprovals' && <AdminApprovalsSection />}
        {guarantorSection === 'editProfile' && <EditProfileSection />}
        {guarantorSection === 'newTermDialog' && <NewTermDialogSection />}
      </div>
    );
  };

  const ActionsDemoWrapper = () => (
    <div className="p-6">
      <div className="ml-8">
        <ActionsDemo
          rows={courses}
          onAdd={handleCreateCourse}
          onApprovePending={handleApprovePending}
          onRejectPending={handleRejectPending}
          onApproveOne={handleApproveOne}
          onRejectOne={handleRejectOne}
        />
      </div>
    </div>
  );

  const tabs = [
    { id: 'Desktop1', name: 'Desktop - 1' },
    { id: 'AppShell', name: 'App Shell' },
    { id: 'TwoColumnForm', name: 'Two Column Form' },
    { id: 'DataTable', name: 'Data Table' },
    { id: 'Guarantor', name: 'Guarantor' },
    { id: 'ActionsDemo', name: 'Actions Demo' },
    { id: 'Courses', name: 'Courses' },
  ];

  const renderContent = () => {
    switch (activeTab) {
      case 'Desktop1':
        return <Desktop1 />;
      case 'AppShell':
        return <AppShell />;
      case 'TwoColumnForm':
        return (
          <TwoColumnForm
            showFormSuccess={showFormSuccess}
            formData={formData}
            onFormChange={handleFormChange}
            onFormSave={handleFormSave}
            onFormReset={handleFormReset}
          />
        );
      case 'DataTable':
        return <DataTable />;
      case 'Guarantor':
        return <Guarantor />;
      case 'ActionsDemo':
        return <ActionsDemoWrapper />;
      case 'Courses':
        return <Courses />;
      default:
        return <Desktop1 />;
    }
  };

  return (
    <div className="flex min-h-screen bg-gray-100">
      <aside className="w-64 bg-blue-600 text-white">
        <nav className="p-4">
          <h1 className="text-lg font-bold mb-6">WIS2 Components</h1>
          <div className="space-y-2">
            {tabs.map((tab) => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={`w-full text-left px-4 py-3 rounded-lg transition-colors ${
                  activeTab === tab.id ? 'bg-blue-700 text-white' : 'text-blue-100 hover:bg-blue-500'
                }`}
              >
                {tab.name}
              </button>
            ))}
          </div>
        </nav>
      </aside>
      <main className="flex-1">{renderContent()}</main>
    </div>
  );
}

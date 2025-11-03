import React from 'react';

export default function ActionsDemo({ rows, onAdd, onApprovePending, onRejectPending, onApproveOne, onRejectOne }) {
  return (
    <div className="bg-white border border-gray-200 rounded-lg p-6 space-y-4">
      <div className="flex flex-wrap gap-2">
        <button
          onClick={onAdd}
          className="px-3 py-2 text-sm text-white bg-blue-600 rounded-md hover:bg-blue-700"
        >
          Add
        </button>
        <button
          onClick={onApprovePending}
          className="px-3 py-2 text-sm text-white bg-green-600 rounded-md hover:bg-green-700"
        >
          Approve Pending
        </button>
        <button
          onClick={onRejectPending}
          className="px-3 py-2 text-sm text-white bg-red-600 rounded-md hover:bg-red-700"
        >
          Reject Pending
        </button>
      </div>

      <table className="w-full text-sm">
        <thead className="bg-gray-50">
          <tr>
            <th className="px-4 py-2 text-left text-xs font-medium text-gray-500 uppercase">Course</th>
            <th className="px-4 py-2 text-left text-xs font-medium text-gray-500 uppercase">Status</th>
            <th className="px-4 py-2 text-left text-xs font-medium text-gray-500 uppercase">Actions</th>
          </tr>
        </thead>
        <tbody>
          {rows.map((row) => (
            <tr key={row.id} className="border-t border-gray-200">
              <td className="px-4 py-2 text-gray-900">{row.title}</td>
              <td className="px-4 py-2 capitalize text-gray-600">{row.status}</td>
              <td className="px-4 py-2">
                <div className="flex gap-2">
                  <button
                    onClick={() => onApproveOne(row.id)}
                    className="px-2 py-1 text-xs border border-green-600 text-green-700 rounded hover:bg-green-50"
                  >
                    ? Approve
                  </button>
                  <button
                    onClick={() => onRejectOne(row.id)}
                    className="px-2 py-1 text-xs border border-red-600 text-red-700 rounded hover:bg-red-50"
                  >
                    ? Reject
                  </button>
                </div>
              </td>
            </tr>
          ))}
          {rows.length === 0 && (
            <tr>
              <td colSpan={3} className="px-4 py-6 text-center text-gray-500">
                No courses available
              </td>
            </tr>
          )}
        </tbody>
      </table>
    </div>
  );
}

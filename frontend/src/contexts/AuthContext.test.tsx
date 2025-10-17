import React, { ReactNode } from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import '@testing-library/jest-dom';
import { AuthProvider, useAuth } from './AuthContext';

// Mock the API service
jest.mock('../services/api', () => ({
  getCurrentUser: jest.fn(),
}));

import { getCurrentUser } from '../services/api';

const mockGetCurrentUser = getCurrentUser as jest.MockedFunction<typeof getCurrentUser>;

// Test component that uses the auth context
const TestComponent: React.FC = () => {
  const { user, login, logout, isAuthenticated, loading } = useAuth();

  if (loading) {
    return <div>Loading...</div>;
  }

  return (
    <div>
      <div data-testid="auth-status">
        {isAuthenticated ? 'Authenticated' : 'Not Authenticated'}
      </div>
      <div data-testid="user-info">
        {user ? `User: ${user.name} (${user.email})` : 'No user'}
      </div>
      <button onClick={() => login({
        id: '1',
        name: 'Test User',
        email: 'test@example.com',
        bio: '',
        avatarUrl: '',
        preferences: {},
        privacySettings: {},
        createdAt: '2025-01-01T00:00:00Z',
        updatedAt: '2025-01-01T00:00:00Z',
        isActive: true
      }, 'test-token')}>
        Login
      </button>
      <button onClick={logout}>Logout</button>
    </div>
  );
};

describe('AuthContext', () => {
  beforeEach(() => {
    jest.clearAllMocks();
    // Clear localStorage
    localStorage.clear();
  });

  it('provides initial unauthenticated state', async () => {
    mockGetCurrentUser.mockRejectedValueOnce(new Error('Not authenticated'));

    render(
      <AuthProvider>
        <TestComponent />
      </AuthProvider>
    );

    // Wait for the auth check to complete
    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Not Authenticated');
    });
    expect(screen.getByTestId('user-info')).toHaveTextContent('No user');
  });

  it('loads user from API on mount if authenticated', async () => {
    const mockUser = {
      id: '1',
      name: 'Existing User',
      email: 'existing@example.com',
      bio: '',
      avatarUrl: '',
      preferences: {},
      privacySettings: {},
      createdAt: '2025-01-01T00:00:00Z',
      updatedAt: '2025-01-01T00:00:00Z',
      isActive: true
    };

    mockGetCurrentUser.mockResolvedValueOnce(mockUser);

    render(
      <AuthProvider>
        <TestComponent />
      </AuthProvider>
    );

    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Authenticated');
      expect(screen.getByTestId('user-info')).toHaveTextContent('User: Existing User (existing@example.com)');
    });

    expect(mockGetCurrentUser).toHaveBeenCalledTimes(1);
  });

  it('handles login correctly', async () => {
    mockGetCurrentUser.mockRejectedValueOnce(new Error('Not authenticated'));

    // Mock localStorage
    const mockSetItem = jest.spyOn(Storage.prototype, 'setItem');
    mockSetItem.mockImplementation(() => {});

    render(
      <AuthProvider>
        <TestComponent />
      </AuthProvider>
    );

    // Wait for initial load
    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Not Authenticated');
    });

    // Click login button
    fireEvent.click(screen.getByText('Login'));

    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Authenticated');
      expect(screen.getByTestId('user-info')).toHaveTextContent('User: Test User (test@example.com)');
    });

    expect(mockSetItem).toHaveBeenCalledWith('authToken', 'test-token');
    expect(mockSetItem).toHaveBeenCalledWith('user', expect.any(String));

    mockSetItem.mockRestore();
  });

  it('handles logout correctly', async () => {
    mockGetCurrentUser.mockRejectedValueOnce(new Error('Not authenticated'));

    // Mock localStorage
    const mockRemoveItem = jest.spyOn(Storage.prototype, 'removeItem');
    mockRemoveItem.mockImplementation(() => {});

    render(
      <AuthProvider>
        <TestComponent />
      </AuthProvider>
    );

    // Wait for initial load
    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Not Authenticated');
    });

    // Login first
    fireEvent.click(screen.getByText('Login'));

    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Authenticated');
    });

    // Now logout
    fireEvent.click(screen.getByText('Logout'));

    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Not Authenticated');
      expect(screen.getByTestId('user-info')).toHaveTextContent('No user');
    });

    expect(mockRemoveItem).toHaveBeenCalledWith('user');
    expect(mockRemoveItem).toHaveBeenCalledWith('authToken');

    mockRemoveItem.mockRestore();
  });

  it('handles API errors gracefully', async () => {
    mockGetCurrentUser.mockRejectedValueOnce(new Error('API Error'));

    render(
      <AuthProvider>
        <TestComponent />
      </AuthProvider>
    );

    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Not Authenticated');
    });

    // Should not crash and should show unauthenticated state
    expect(screen.getByTestId('user-info')).toHaveTextContent('No user');
  });

  it('throws error when useAuth is used outside provider', () => {
    const consoleSpy = jest.spyOn(console, 'error').mockImplementation(() => {});

    expect(() => {
      render(<TestComponent />);
    }).toThrow('useAuth must be used within an AuthProvider');

    consoleSpy.mockRestore();
  });

  it('persists user data in localStorage', async () => {
    mockGetCurrentUser.mockRejectedValueOnce(new Error('Not authenticated'));

    const mockSetItem = jest.spyOn(Storage.prototype, 'setItem');
    mockSetItem.mockImplementation(() => {});

    render(
      <AuthProvider>
        <TestComponent />
      </AuthProvider>
    );

    // Wait for initial load
    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Not Authenticated');
    });

    // Login
    fireEvent.click(screen.getByText('Login'));

    await waitFor(() => {
      expect(screen.getByTestId('auth-status')).toHaveTextContent('Authenticated');
    });

    // Check that localStorage.setItem was called with user data
    expect(mockSetItem).toHaveBeenCalledWith('user', expect.stringContaining('Test User'));

    mockSetItem.mockRestore();
  });
});
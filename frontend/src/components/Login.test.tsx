import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import '@testing-library/jest-dom';
import Login from './Login';

// Mock the API service
jest.mock('../services/api', () => ({
  login: jest.fn(),
}));

import { login } from '../services/api';

const mockLogin = login as jest.MockedFunction<typeof login>;
const mockOnLogin = jest.fn();
const mockOnSwitchToRegister = jest.fn();

describe('Login Component', () => {
  beforeEach(() => {
    jest.clearAllMocks();
  });

  it('renders login form correctly', () => {
    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    expect(screen.getByText('Login to RecipeForADisaster')).toBeInTheDocument();
    expect(screen.getByLabelText('Email')).toBeInTheDocument();
    expect(screen.getByLabelText('Password')).toBeInTheDocument();
    expect(screen.getByRole('button', { name: 'Login' })).toBeInTheDocument();
    expect(screen.getByText("Don't have an account?")).toBeInTheDocument();
  });

  it('updates form fields when user types', async () => {
    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    const emailInput = screen.getByLabelText('Email');
    const passwordInput = screen.getByLabelText('Password');

    fireEvent.change(emailInput, { target: { value: 'test@example.com' } });
    fireEvent.change(passwordInput, { target: { value: 'password123' } });

    expect(emailInput).toHaveValue('test@example.com');
    expect(passwordInput).toHaveValue('password123');
  });

  it('calls onLogin with user data on successful login', async () => {
    const mockUser = {
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
    };
    const mockToken = 'mock-jwt-token';

    mockLogin.mockResolvedValueOnce({
      success: true,
      message: 'Login successful',
      data: { token: mockToken, user: mockUser }
    });

    // Mock localStorage
    const mockSetItem = jest.spyOn(Storage.prototype, 'setItem');
    mockSetItem.mockImplementation(() => {});

    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    const emailInput = screen.getByLabelText('Email');
    const passwordInput = screen.getByLabelText('Password');
    const submitButton = screen.getByRole('button', { name: 'Login' });

    fireEvent.change(emailInput, { target: { value: 'test@example.com' } });
    fireEvent.change(passwordInput, { target: { value: 'password123' } });
    fireEvent.click(submitButton);

    await waitFor(() => {
      expect(mockLogin).toHaveBeenCalledWith('test@example.com', 'password123');
      expect(mockSetItem).toHaveBeenCalledWith('authToken', mockToken);
      expect(mockOnLogin).toHaveBeenCalledWith(mockUser);
    });

    mockSetItem.mockRestore();
  });

  it('shows error message on login failure', async () => {
    mockLogin.mockResolvedValueOnce({
      success: false,
      message: 'Invalid credentials'
    });

    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    const emailInput = screen.getByLabelText('Email');
    const passwordInput = screen.getByLabelText('Password');
    const submitButton = screen.getByRole('button', { name: 'Login' });

    fireEvent.change(emailInput, { target: { value: 'wrong@example.com' } });
    fireEvent.change(passwordInput, { target: { value: 'wrongpassword' } });
    fireEvent.click(submitButton);

    await waitFor(() => {
      expect(screen.getByText('Invalid credentials')).toBeInTheDocument();
    });
  });

  it('shows loading state during login', async () => {
    mockLogin.mockImplementation(() => new Promise(resolve => setTimeout(resolve, 100)));

    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    const emailInput = screen.getByLabelText('Email');
    const passwordInput = screen.getByLabelText('Password');
    const submitButton = screen.getByRole('button', { name: 'Login' });

    fireEvent.change(emailInput, { target: { value: 'test@example.com' } });
    fireEvent.change(passwordInput, { target: { value: 'password123' } });
    fireEvent.click(submitButton);

    expect(screen.getByText('Logging in...')).toBeInTheDocument();
    expect(submitButton).toBeDisabled();
  });

  it('calls onSwitchToRegister when register link is clicked', async () => {
    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    const registerLink = screen.getByText('Register here');
    fireEvent.click(registerLink);

    expect(mockOnSwitchToRegister).toHaveBeenCalledTimes(1);
  });

  it('validates required fields', async () => {
    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    const submitButton = screen.getByRole('button', { name: 'Login' });
    fireEvent.click(submitButton);

    // Should show error message for empty fields
    await waitFor(() => {
      expect(screen.getByText('Invalid email or password')).toBeInTheDocument();
    });

    // onLogin should not be called
    expect(mockOnLogin).not.toHaveBeenCalled();
  });

  it('shows loading state during login', async () => {
    mockLogin.mockImplementation(() => new Promise(resolve => setTimeout(resolve, 100)));

    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    const emailInput = screen.getByLabelText('Email');
    const passwordInput = screen.getByLabelText('Password');
    const submitButton = screen.getByRole('button', { name: 'Login' });

    fireEvent.change(emailInput, { target: { value: 'test@example.com' } });
    fireEvent.change(passwordInput, { target: { value: 'password123' } });
    fireEvent.click(submitButton);

    expect(screen.getByText('Logging in...')).toBeInTheDocument();
    expect(submitButton).toBeDisabled();
  });

  it('calls onSwitchToRegister when register link is clicked', async () => {
    render(
      <Login onLogin={mockOnLogin} onSwitchToRegister={mockOnSwitchToRegister} />
    );

    const registerLink = screen.getByText('Register here');
    fireEvent.click(registerLink);

    expect(mockOnSwitchToRegister).toHaveBeenCalledTimes(1);
  });
});
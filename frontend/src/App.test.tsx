import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import '@testing-library/jest-dom';
import App from './App';

// Mock the entire API module
jest.mock('./services/api', () => ({
  login: jest.fn(),
  register: jest.fn(),
  getCurrentUser: jest.fn(),
  updateCurrentUser: jest.fn(),
  getCollections: jest.fn(),
  recipeApi: {
    getAllRecipes: jest.fn(),
    searchRecipes: jest.fn(),
    getRecipesByCategory: jest.fn(),
    getRecipesByType: jest.fn(),
  },
}));

import {
  login,
  register,
  getCurrentUser,
  updateCurrentUser,
  getCollections,
  recipeApi
} from './services/api';

const mockLogin = login as jest.MockedFunction<typeof login>;
const mockRegister = register as jest.MockedFunction<typeof register>;
const mockGetCurrentUser = getCurrentUser as jest.MockedFunction<typeof getCurrentUser>;
const mockUpdateCurrentUser = updateCurrentUser as jest.MockedFunction<typeof updateCurrentUser>;
const mockGetCollections = getCollections as jest.MockedFunction<typeof getCollections>;
const mockGetAllRecipes = recipeApi.getAllRecipes as jest.MockedFunction<typeof recipeApi.getAllRecipes>;

describe('Authentication Integration Tests', () => {
  beforeEach(() => {
    jest.clearAllMocks();
    localStorage.clear();

    // Default mocks
    mockGetCurrentUser.mockRejectedValue(new Error('Not authenticated'));
    mockGetAllRecipes.mockResolvedValue([]);
    mockGetCollections.mockResolvedValue([]);
  });

  describe('Complete Authentication Flow', () => {
    it('allows user to register and then access the application', async () => {
      const mockUser = {
        id: '1',
        name: 'John Doe',
        email: 'john@example.com',
        bio: '',
        avatarUrl: '',
        preferences: {},
        privacySettings: {},
        createdAt: '2025-01-01T00:00:00Z',
        updatedAt: '2025-01-01T00:00:00Z',
        isActive: true
      };

      mockRegister.mockResolvedValue({
        success: true,
        message: 'Registration successful',
        data: { user: mockUser, token: 'test-token' }
      });

      render(<App />);

      // Wait for loading to complete and welcome screen to appear
      await waitFor(() => {
        expect(screen.getByText('Welcome to RecipeForADisaster')).toBeInTheDocument();
      });
      expect(screen.getByText('Get Started - Create Account')).toBeInTheDocument();

      // Click register button
      fireEvent.click(screen.getByText('Get Started - Create Account'));

      // Fill out registration form
      fireEvent.change(screen.getByLabelText('Full Name'), { target: { value: 'John Doe' } });
      fireEvent.change(screen.getByLabelText('Email'), { target: { value: 'john@example.com' } });
      fireEvent.change(screen.getByLabelText('Password'), { target: { value: 'password123' } });
      fireEvent.change(screen.getByLabelText('Confirm Password'), { target: { value: 'password123' } });

      // Submit registration
      const form = screen.getByRole('form');
      fireEvent.submit(form);

      // Should redirect to authenticated app
      await waitFor(() => {
        expect(screen.getByText('Welcome, John Doe')).toBeInTheDocument();
      });
      expect(screen.getByText('Recipes')).toBeInTheDocument();
      expect(screen.getByText('Collections')).toBeInTheDocument();
      expect(screen.getByText('Profile')).toBeInTheDocument();
    });

    it('allows user to login and access the application', async () => {
      // Clear all mocks and localStorage for this test
      jest.clearAllMocks();
      localStorage.clear();

      // Initial auth check should fail
      mockGetCurrentUser.mockRejectedValueOnce(new Error('Not authenticated'));
      mockGetAllRecipes.mockResolvedValue([]);
      mockGetCollections.mockResolvedValue([]);

      const mockUser = {
        id: '1',
        name: 'Jane Smith',
        email: 'jane@example.com',
        bio: '',
        avatarUrl: '',
        preferences: {},
        privacySettings: {},
        createdAt: '2025-01-01T00:00:00Z',
        updatedAt: '2025-01-01T00:00:00Z',
        isActive: true
      };

      const mockToken = 'jwt-token-456';

      mockLogin.mockResolvedValueOnce({
        success: true,
        message: 'Login successful',
        data: { token: mockToken, user: mockUser }
      });

      // After login, getCurrentUser should succeed
      mockGetCurrentUser.mockResolvedValueOnce(mockUser);
      mockGetAllRecipes.mockResolvedValueOnce([]);

      render(<App />);

      // Wait for loading to complete
      await waitFor(() => {
        expect(screen.queryByText('Loading...')).not.toBeInTheDocument();
      });

      // Wait for welcome screen to appear
      await waitFor(() => {
        expect(screen.getByText('Welcome to RecipeForADisaster')).toBeInTheDocument();
      });

      // Click login button
      fireEvent.click(screen.getByText('Already have an account? Login'));

      // Should show login modal
      expect(screen.getByText('Login to RecipeForADisaster')).toBeInTheDocument();

      // Fill out login form
      fireEvent.change(screen.getByLabelText('Email'), { target: { value: 'jane@example.com' } });
      fireEvent.change(screen.getByLabelText('Password'), { target: { value: 'password123' } });

      // Submit login
      const form = screen.getByRole('form');
      fireEvent.submit(form);

      // Should authenticate and show main app
      await waitFor(() => {
        expect(screen.getByText('RecipeForADisaster')).toBeInTheDocument();
        expect(screen.getByText('Welcome, Jane Smith')).toBeInTheDocument();
      });
    });

    it('handles authentication errors gracefully', async () => {
      mockLogin.mockResolvedValueOnce({
        success: false,
        message: 'Invalid credentials'
      });

      render(<App />);

      // Wait for loading to complete
      await waitFor(() => {
        expect(screen.getByText('Welcome to RecipeForADisaster')).toBeInTheDocument();
      });

      // Click login button
      fireEvent.click(screen.getByText('Already have an account? Login'));

      // Fill out login form with wrong credentials
      fireEvent.change(screen.getByLabelText('Email'), { target: { value: 'wrong@example.com' } });
      fireEvent.change(screen.getByLabelText('Password'), { target: { value: 'wrongpassword' } });

      // Submit login
      const submitButtons = screen.getAllByRole('button', { name: 'Login' });
      fireEvent.click(submitButtons[1]); // The submit button in the form

      // Should show error message
      await waitFor(() => {
        expect(screen.getByText('Invalid credentials')).toBeInTheDocument();
      });

      // Should still show login modal
      expect(screen.getByText('Login to RecipeForADisaster')).toBeInTheDocument();
    });

    it('allows switching between login and register forms', async () => {
      render(<App />);

      // Wait for loading to complete
      await waitFor(() => {
        expect(screen.getByText('Welcome to RecipeForADisaster')).toBeInTheDocument();
      });

      // Click login button
      fireEvent.click(screen.getByText('Already have an account? Login'));

      // Should show login modal
      expect(screen.getByText('Login to RecipeForADisaster')).toBeInTheDocument();

      // Click register link
      fireEvent.click(screen.getByText('Register here'));

      // Should show register modal
      expect(screen.getByText('Join RecipeForADisaster')).toBeInTheDocument();

      // Click login link
      fireEvent.click(screen.getByText('Login here'));

      // Should show login modal again
      expect(screen.getByText('Login to RecipeForADisaster')).toBeInTheDocument();
    });

    it('persists authentication across app reloads', async () => {
      const mockUser = {
        id: '1',
        name: 'Persistent User',
        email: 'persistent@example.com',
        bio: '',
        avatarUrl: '',
        preferences: {},
        privacySettings: {},
        createdAt: '2025-01-01T00:00:00Z',
        updatedAt: '2025-01-01T00:00:00Z',
        isActive: true
      };

      // Mock successful login
      mockLogin.mockResolvedValueOnce({
        success: true,
        message: 'Login successful',
        data: { token: 'test-token', user: mockUser }
      });

      render(<App />);

      // Wait for loading to complete
      await waitFor(() => {
        expect(screen.getByText('Welcome to RecipeForADisaster')).toBeInTheDocument();
      });

      // Login process
      fireEvent.click(screen.getByText('Already have an account? Login'));
      fireEvent.change(screen.getByLabelText('Email'), { target: { value: 'persistent@example.com' } });
      fireEvent.change(screen.getByLabelText('Password'), { target: { value: 'password123' } });
      fireEvent.submit(screen.getAllByRole('form')[0]);

      // Should authenticate and store in localStorage
      await waitFor(() => {
        expect(localStorage.getItem('authToken')).toBe('test-token');
        expect(localStorage.getItem('user')).toBe(JSON.stringify(mockUser));
      });
    });

    it('allows user to logout and return to welcome screen', async () => {
      const mockUser = {
        id: '1',
        name: 'Logout User',
        email: 'logout@example.com',
        bio: '',
        avatarUrl: '',
        preferences: {},
        privacySettings: {},
        createdAt: '2025-01-01T00:00:00Z',
        updatedAt: '2025-01-01T00:00:00Z',
        isActive: true
      };

      mockGetCurrentUser.mockResolvedValueOnce(mockUser);
      mockGetAllRecipes.mockResolvedValueOnce([]);

      render(<App />);

      // Should show authenticated app
      await waitFor(() => {
        expect(screen.getByText('Welcome, Logout User')).toBeInTheDocument();
      });

      // Click logout button
      fireEvent.click(screen.getByText('Logout'));

      // Should return to welcome screen
      await waitFor(() => {
        expect(screen.getByText('Welcome to RecipeForADisaster')).toBeInTheDocument();
      });

      // localStorage should be cleared
      expect(localStorage.getItem('authToken')).toBeNull();
      expect(localStorage.getItem('user')).toBeNull();
    });
  });

  describe('Protected Features', () => {
    it('shows profile and collections tabs only when authenticated', async () => {
      const mockUser = {
        id: '1',
        name: 'Profile User',
        email: 'profile@example.com',
        bio: '',
        avatarUrl: '',
        preferences: {},
        privacySettings: {},
        createdAt: '2025-01-01T00:00:00Z',
        updatedAt: '2025-01-01T00:00:00Z',
        isActive: true
      };

      mockGetCurrentUser.mockResolvedValueOnce(mockUser);
      mockGetAllRecipes.mockResolvedValueOnce([]);

      render(<App />);

      await waitFor(() => {
        expect(screen.getByText('Welcome, Profile User')).toBeInTheDocument();
      });

      // Should show all navigation tabs
      expect(screen.getByText('Recipes')).toBeInTheDocument();
      expect(screen.getByText('Collections')).toBeInTheDocument();
      expect(screen.getByText('Profile')).toBeInTheDocument();
    });

    it('allows navigation between authenticated features', async () => {
      const mockUser = {
        id: '1',
        name: 'Navigation User',
        email: 'nav@example.com',
        bio: '',
        avatarUrl: '',
        preferences: {},
        privacySettings: {},
        createdAt: '2025-01-01T00:00:00Z',
        updatedAt: '2025-01-01T00:00:00Z',
        isActive: true
      };

      mockGetCurrentUser.mockResolvedValue(mockUser);
      mockGetAllRecipes.mockResolvedValue([]);
      mockGetCollections.mockResolvedValue([]);

      render(<App />);

      await waitFor(() => {
        expect(screen.getByText('Welcome, Navigation User')).toBeInTheDocument();
      });

      // Click Collections tab
      fireEvent.click(screen.getByText('Collections'));

      // Should show collections view
      await waitFor(() => {
        expect(screen.getByText('My Collections')).toBeInTheDocument();
      });

      // Click Profile tab
      fireEvent.click(screen.getByText('Profile'));

      // Should show profile view
      await waitFor(() => {
        expect(screen.getByText('My Profile')).toBeInTheDocument();
      });

      // Click Recipes tab
      fireEvent.click(screen.getByText('Recipes'));

      // Should show recipes list
      await waitFor(() => {
        expect(screen.getByText('Add New Recipe')).toBeInTheDocument();
      });
    });
  });
});
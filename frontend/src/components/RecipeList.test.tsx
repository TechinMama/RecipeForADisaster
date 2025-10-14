import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import '@testing-library/jest-dom';
import RecipeList from './RecipeList';
import { Recipe } from '../types/Recipe';

// Mock the API service
jest.mock('../services/api', () => ({
  recipeApi: {
    getAllRecipes: jest.fn(),
    searchRecipes: jest.fn(),
    getAllRecipesByCategory: jest.fn(),
    getAllRecipesByType: jest.fn(),
  },
}));

import { recipeApi } from '../services/api';

const mockRecipeApi = recipeApi as jest.Mocked<typeof recipeApi>;

describe('RecipeList', () => {
  const mockRecipes: Recipe[] = [
    {
      title: 'Chocolate Chip Cookies',
      ingredients: 'flour, butter, sugar, chocolate chips',
      instructions: 'Mix ingredients, bake at 350°F for 12 minutes',
      servingSize: '24 cookies',
      cookTime: '12-15 minutes',
      category: 'Dessert',
      type: 'Cookies',
    },
    {
      title: 'Pasta Carbonara',
      ingredients: 'pasta, eggs, bacon, cheese',
      instructions: 'Boil pasta, mix with sauce',
      servingSize: '4 servings',
      cookTime: '20 minutes',
      category: 'Main',
      type: 'Pasta',
    },
  ];

  const mockProps = {
    onEdit: jest.fn(),
    onView: jest.fn(),
    refreshTrigger: 0,
  };

  beforeEach(() => {
    jest.clearAllMocks();
  });

  test('renders loading state initially', () => {
    mockRecipeApi.getAllRecipes.mockImplementation(() => new Promise(() => {})); // Never resolves

    render(<RecipeList {...mockProps} />);

    expect(screen.getByText('Loading recipes...')).toBeInTheDocument();
  });

  test('renders recipes when loaded successfully', async () => {
    mockRecipeApi.getAllRecipes.mockResolvedValue(mockRecipes);

    render(<RecipeList {...mockProps} />);

    await waitFor(() => {
      expect(screen.getByText('Chocolate Chip Cookies')).toBeInTheDocument();
      expect(screen.getByText('Pasta Carbonara')).toBeInTheDocument();
    });
  });

  test('displays recipe details correctly', async () => {
    mockRecipeApi.getAllRecipes.mockResolvedValue([mockRecipes[0]]);

    render(<RecipeList {...mockProps} />);

    await waitFor(() => {
      expect(screen.getByText('Chocolate Chip Cookies')).toBeInTheDocument();
      // Check for category and type in the recipe card specifically
      const recipeCard = screen.getByText('Chocolate Chip Cookies').closest('.recipe-card');
      expect(recipeCard).toHaveTextContent('Dessert');
      expect(recipeCard).toHaveTextContent('Cookies');
      expect(screen.getByText('24 cookies')).toBeInTheDocument();
      expect(screen.getByText('12-15 minutes')).toBeInTheDocument();
    });
  });

  test('calls onEdit when edit button is clicked', async () => {
    mockRecipeApi.getAllRecipes.mockResolvedValue([mockRecipes[0]]);

    render(<RecipeList {...mockProps} />);

    await waitFor(() => {
      expect(screen.getByText('Chocolate Chip Cookies')).toBeInTheDocument();
    });

    const editButton = screen.getByRole('button', { name: /edit/i });
    fireEvent.click(editButton);

    expect(mockProps.onEdit).toHaveBeenCalledWith(mockRecipes[0]);
  });

  test('calls onView when view button is clicked', async () => {
    mockRecipeApi.getAllRecipes.mockResolvedValue([mockRecipes[0]]);

    render(<RecipeList {...mockProps} />);

    await waitFor(() => {
      expect(screen.getByText('Chocolate Chip Cookies')).toBeInTheDocument();
    });

    const viewButton = screen.getByRole('button', { name: /view/i });
    fireEvent.click(viewButton);

    expect(mockProps.onView).toHaveBeenCalledWith(mockRecipes[0]);
  });

  test('displays error message when API fails', async () => {
    mockRecipeApi.getAllRecipes.mockRejectedValue(new Error('API Error'));

    render(<RecipeList {...mockProps} />);

    await waitFor(() => {
      expect(screen.getByText('Failed to load recipes')).toBeInTheDocument();
    });
  });

  test('shows empty state when no recipes', async () => {
    mockRecipeApi.getAllRecipes.mockResolvedValue([]);

    render(<RecipeList {...mockProps} />);

    await waitFor(() => {
      expect(screen.getByText('No recipes found. Try adjusting your filters or add a new recipe.')).toBeInTheDocument();
    });
  });

  test('refreshes when refreshTrigger changes', async () => {
    mockRecipeApi.getAllRecipes.mockResolvedValue(mockRecipes);

    const { rerender } = render(<RecipeList {...mockProps} refreshTrigger={0} />);

    await waitFor(() => {
      expect(mockRecipeApi.getAllRecipes).toHaveBeenCalledTimes(1);
    });

    // Change refresh trigger
    rerender(<RecipeList {...mockProps} refreshTrigger={1} />);

    await waitFor(() => {
      expect(mockRecipeApi.getAllRecipes).toHaveBeenCalledTimes(2);
    });
  });
});
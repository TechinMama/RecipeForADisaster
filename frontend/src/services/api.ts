import axios from 'axios';
import { Recipe } from '../types/Recipe';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:8080/api';

const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
});

export const recipeApi = {
  // Get all recipes
  getAllRecipes: async (): Promise<Recipe[]> => {
    const response = await api.get('/recipes');
    return response.data.data.recipes;
  },

  // Search recipes by title
  searchRecipes: async (query: string): Promise<Recipe[]> => {
    const response = await api.get(`/recipes/search?q=${encodeURIComponent(query)}`);
    return response.data.data.recipes;
  },

  // Get recipes by category
  getRecipesByCategory: async (category: string): Promise<Recipe[]> => {
    const response = await api.get(`/recipes/categories/${encodeURIComponent(category)}`);
    return response.data.data.recipes;
  },

  // Get recipes by type
  getRecipesByType: async (type: string): Promise<Recipe[]> => {
    const response = await api.get(`/recipes/types/${encodeURIComponent(type)}`);
    return response.data.data.recipes;
  },

  // Add new recipe
  addRecipe: async (recipe: Recipe): Promise<{ success: boolean; message: string }> => {
    const response = await api.post('/recipes', recipe);
    return response.data;
  },

  // Update recipe
  updateRecipe: async (oldTitle: string, recipe: Recipe): Promise<{ success: boolean; message: string }> => {
    const response = await api.put(`/recipes/${encodeURIComponent(oldTitle)}`, recipe);
    return response.data;
  },

  // Delete recipe
  deleteRecipe: async (title: string): Promise<{ success: boolean; message: string }> => {
    const response = await api.delete(`/recipes/${encodeURIComponent(title)}`);
    return response.data;
  },

  // Generate recipe with AI
  generateRecipe: async (prompt: string, count?: number): Promise<any> => {
    const response = await api.post('/recipes/generate', { prompt, count: count || 1 });
    return response.data;
  },

  // Check AI service status
  getAiStatus: async (): Promise<any> => {
    const response = await api.get('/ai/status');
    return response.data;
  },

  // Health check
  healthCheck: async (): Promise<any> => {
    const response = await api.get('/health');
    return response.data;
  },
};

export default api;
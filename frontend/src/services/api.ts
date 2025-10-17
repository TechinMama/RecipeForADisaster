import axios from 'axios';
import { Recipe, User, Collection } from '../types/Recipe';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:8080/api';

const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
});

// Add request interceptor to include auth token
api.interceptors.request.use(
  (config) => {
    const token = localStorage.getItem('authToken');
    if (token) {
      config.headers.Authorization = `Bearer ${token}`;
    }
    return config;
  },
  (error) => {
    return Promise.reject(error);
  }
);

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

  // Advanced search with multiple criteria
  advancedSearchRecipes: async (criteria: {
    query?: string;
    category?: string;
    type?: string;
    cookTimeMax?: string;
    servingSizeMin?: string;
    servingSizeMax?: string;
    ingredient?: string;
    sortBy?: string;
    sortOrder?: string;
  }): Promise<Recipe[]> => {
    const params = new URLSearchParams();
    Object.entries(criteria).forEach(([key, value]) => {
      if (value) params.set(key, value);
    });
    const response = await api.get(`/recipes/advanced-search?${params.toString()}`);
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

  // Get current user profile
  getCurrentUser: async (): Promise<User> => {
    const response = await api.get('/auth/me');
    return response.data.data;
  },

  // Update current user profile
  updateCurrentUser: async (userData: Partial<User>): Promise<{ success: boolean; message: string }> => {
    const response = await api.put('/auth/me', userData);
    return response.data;
  },

  // Get user's collections
  getCollections: async (): Promise<Collection[]> => {
    const response = await api.get('/collections');
    return response.data.data.collections;
  },

  // Create new collection
  createCollection: async (collection: Omit<Collection, 'id' | 'userId' | 'createdAt' | 'updatedAt'>): Promise<{ success: boolean; message: string; data: Collection }> => {
    const response = await api.post('/collections', collection);
    return response.data;
  },

  // Get specific collection
  getCollection: async (id: string): Promise<Collection> => {
    const response = await api.get(`/collections/${id}`);
    return response.data.data.collection;
  },

  // Update collection
  updateCollection: async (id: string, collection: Partial<Collection>): Promise<{ success: boolean; message: string }> => {
    const response = await api.put(`/collections/${id}`, collection);
    return response.data;
  },

  // Delete collection
  deleteCollection: async (id: string): Promise<{ success: boolean; message: string }> => {
    const response = await api.delete(`/collections/${id}`);
    return response.data;
  },

  // Add recipe to collection
  addRecipeToCollection: async (collectionId: string, recipeId: string): Promise<{ success: boolean; message: string }> => {
    const response = await api.post(`/collections/${collectionId}/recipes/${recipeId}`);
    return response.data;
  },

  // Remove recipe from collection
  removeRecipeFromCollection: async (collectionId: string, recipeId: string): Promise<{ success: boolean; message: string }> => {
    const response = await api.delete(`/collections/${collectionId}/recipes/${recipeId}`);
    return response.data;
  },

  // Login user
  login: async (email: string, password: string): Promise<{ success: boolean; message: string; data?: { token: string; user: User } }> => {
    const response = await api.post('/auth/login', { email, password });
    return response.data;
  },

  // Register new user
  register: async (userData: { name: string; email: string; password: string }): Promise<{ success: boolean; message: string; data?: { token: string; user: User } }> => {
    const response = await api.post('/auth/register', userData);
    return response.data;
  },

  // Logout user
  logout: async (): Promise<{ success: boolean; message: string }> => {
    const response = await api.post('/auth/logout');
    return response.data;
  },
};

// Export individual functions for convenience
export const {
  getAllRecipes,
  searchRecipes,
  advancedSearchRecipes,
  getRecipesByCategory,
  getRecipesByType,
  addRecipe,
  updateRecipe,
  deleteRecipe,
  generateRecipe,
  getAiStatus,
  healthCheck,
  login,
  register,
  logout,
  getCurrentUser,
  updateCurrentUser,
  getCollections,
  createCollection,
  getCollection,
  updateCollection,
  deleteCollection,
  addRecipeToCollection,
  removeRecipeFromCollection
} = recipeApi;

export default api;
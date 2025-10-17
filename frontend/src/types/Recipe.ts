export interface Recipe {
  title: string;
  ingredients: string;
  instructions: string;
  servingSize: string;
  cookTime: string;
  category: string;
  type: string;
}

export interface User {
  id: string;
  email: string;
  name: string;
  bio: string;
  avatarUrl: string;
  preferences: Record<string, any>;
  privacySettings: Record<string, any>;
  createdAt: string;
  updatedAt: string;
  isActive: boolean;
}

export interface Collection {
  id: string;
  name: string;
  description: string;
  userId: string;
  privacySettings: Record<string, any>;
  createdAt: string;
  updatedAt: string;
}

export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  message?: string;
  error?: string;
}
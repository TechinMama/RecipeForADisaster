export interface Recipe {
  title: string;
  ingredients: string;
  instructions: string;
  servingSize: string;
  cookTime: string;
  category: string;
  type: string;
}

export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  message?: string;
  error?: string;
}
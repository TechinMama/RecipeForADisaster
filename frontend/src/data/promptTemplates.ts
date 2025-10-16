// AI Recipe Generation Prompt Templates and Examples
export interface PromptTemplate {
  id: string;
  name: string;
  description: string;
  template: string;
  category: string;
  difficulty?: 'easy' | 'medium' | 'hard';
  cuisine?: string;
  dietary?: string[];
}

export const PROMPT_TEMPLATES: PromptTemplate[] = [
  {
    id: 'quick-weeknight',
    name: 'Quick Weeknight Dinner',
    description: 'A simple, fast recipe perfect for busy weeknights',
    template: 'Create a simple {cuisine} recipe that can be prepared in under 30 minutes using common pantry ingredients. Focus on {dietary} options with minimal prep work.',
    category: 'Quick & Easy',
    difficulty: 'easy',
    cuisine: 'any',
    dietary: ['vegetarian']
  },
  {
    id: 'healthy-salad',
    name: 'Healthy Salad Bowl',
    description: 'Nutritious and fresh salad with protein and seasonal ingredients',
    template: 'Design a healthy {dietary} salad bowl with seasonal vegetables, lean protein, and a light dressing. Include nutritional highlights and make it satisfying for lunch or dinner.',
    category: 'Healthy',
    difficulty: 'easy',
    cuisine: 'any',
    dietary: ['vegetarian', 'gluten-free']
  },
  {
    id: 'comfort-food',
    name: 'Comfort Food Classic',
    description: 'Hearty, warming dishes that feel like home',
    template: 'Create a comforting {cuisine} dish that warms you from the inside out. Include slow-cooked elements, rich flavors, and simple comfort food ingredients.',
    category: 'Comfort Food',
    difficulty: 'medium',
    cuisine: 'american',
    dietary: []
  },
  {
    id: 'baking-dessert',
    name: 'Baking Project',
    description: 'Sweet treats and baked goods for special occasions',
    template: 'Design a {difficulty} {cuisine} dessert recipe with step-by-step baking instructions. Include tips for success and variations for different skill levels.',
    category: 'Desserts',
    difficulty: 'medium',
    cuisine: 'any',
    dietary: []
  },
  {
    id: 'meal-prep',
    name: 'Meal Prep Friendly',
    description: 'Recipes designed for batch cooking and meal preparation',
    template: 'Create a {dietary} meal prep recipe that makes 4-6 servings and stores well in the refrigerator. Include reheating instructions and nutrition information.',
    category: 'Meal Prep',
    difficulty: 'easy',
    cuisine: 'any',
    dietary: ['vegetarian']
  },
  {
    id: 'gourmet-dinner',
    name: 'Gourmet Dinner Party',
    description: 'Impressive dishes for special occasions and entertaining',
    template: 'Design an elegant {cuisine} dinner party recipe with sophisticated flavors and presentation. Include wine pairings and make-ahead instructions.',
    category: 'Gourmet',
    difficulty: 'hard',
    cuisine: 'french',
    dietary: []
  },
  {
    id: 'kid-friendly',
    name: 'Kid-Friendly Meals',
    description: 'Recipes that children will actually enjoy eating',
    template: 'Create a fun, colorful recipe that kids will love to eat and help prepare. Use familiar flavors with a healthy twist and include fun presentation ideas.',
    category: 'Family',
    difficulty: 'easy',
    cuisine: 'american',
    dietary: []
  },
  {
    id: 'international-cuisine',
    name: 'International Cuisine',
    description: 'Authentic recipes from around the world',
    template: 'Create an authentic {cuisine} recipe with traditional ingredients and cooking techniques. Include cultural context and serving suggestions.',
    category: 'International',
    difficulty: 'medium',
    cuisine: 'italian',
    dietary: []
  }
];

export const PROMPT_EXAMPLES = [
  'A creamy pasta dish with seasonal vegetables and herbs',
  'Spicy Thai curry with coconut milk and fresh basil',
  'Classic chocolate chip cookies with a chewy center',
  'Grilled salmon with lemon herb butter and roasted vegetables',
  'Vegetarian chili with beans, tomatoes, and spices',
  'Homemade pizza with fresh mozzarella and basil',
  'Chicken stir-fry with mixed vegetables and ginger soy sauce',
  'Fresh summer salad with feta cheese and balsamic vinaigrette',
  'Beef stew with root vegetables and herbs',
  'Breakfast burritos with eggs, cheese, and salsa'
];

export const CUISINE_TYPES = [
  'American',
  'Italian',
  'Mexican',
  'Chinese',
  'Japanese',
  'Thai',
  'Indian',
  'French',
  'Mediterranean',
  'Korean',
  'Vietnamese',
  'Greek',
  'Spanish',
  'Moroccan',
  'Lebanese'
];

export const DIETARY_RESTRICTIONS = [
  'Vegetarian',
  'Vegan',
  'Gluten-Free',
  'Dairy-Free',
  'Keto',
  'Paleo',
  'Low-Carb',
  'Nut-Free',
  'Soy-Free'
];

export const DIFFICULTY_LEVELS = [
  { value: 'easy', label: 'Easy (15-30 min)', time: '15-30 minutes' },
  { value: 'medium', label: 'Medium (30-60 min)', time: '30-60 minutes' },
  { value: 'hard', label: 'Advanced (60+ min)', time: '60+ minutes' }
];

export const COOKING_TIME_OPTIONS = [
  'Under 15 minutes',
  '15-30 minutes',
  '30-45 minutes',
  '45-60 minutes',
  '1-2 hours',
  'Over 2 hours'
];
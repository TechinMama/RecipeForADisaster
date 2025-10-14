import React, { useState, useEffect } from 'react';
import { Recipe } from '../types/Recipe';
import { recipeApi } from '../services/api';

interface RecipeListProps {
  onEdit: (recipe: Recipe) => void;
  refreshTrigger: number;
}

const RecipeList: React.FC<RecipeListProps> = ({ onEdit, refreshTrigger }) => {
  const [recipes, setRecipes] = useState<Recipe[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [categoryFilter, setCategoryFilter] = useState('');
  const [typeFilter, setTypeFilter] = useState('');

  const loadRecipes = async () => {
    try {
      setLoading(true);
      let data: Recipe[];

      if (searchQuery) {
        data = await recipeApi.searchRecipes(searchQuery);
      } else if (categoryFilter) {
        data = await recipeApi.getRecipesByCategory(categoryFilter);
      } else if (typeFilter) {
        data = await recipeApi.getRecipesByType(typeFilter);
      } else {
        data = await recipeApi.getAllRecipes();
      }

      setRecipes(data);
      setError(null);
    } catch (err) {
      setError('Failed to load recipes');
      console.error('Error loading recipes:', err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    loadRecipes();
  }, [searchQuery, categoryFilter, typeFilter, refreshTrigger]);

  const handleDelete = async (title: string) => {
    if (window.confirm(`Are you sure you want to delete "${title}"?`)) {
      try {
        await recipeApi.deleteRecipe(title);
        loadRecipes();
      } catch (err) {
        alert('Failed to delete recipe');
        console.error('Error deleting recipe:', err);
      }
    }
  };

  const clearFilters = () => {
    setSearchQuery('');
    setCategoryFilter('');
    setTypeFilter('');
  };

  if (loading) return <div className="loading">Loading recipes...</div>;
  if (error) return <div className="error">{error}</div>;

  return (
    <div className="recipe-list">
      <div className="filters">
        <input
          type="text"
          placeholder="Search recipes..."
          value={searchQuery}
          onChange={(e) => setSearchQuery(e.target.value)}
          className="search-input"
        />
        <select
          value={categoryFilter}
          onChange={(e) => setCategoryFilter(e.target.value)}
          className="filter-select"
        >
          <option value="">All Categories</option>
          <option value="Italian">Italian</option>
          <option value="Mexican">Mexican</option>
          <option value="Asian">Asian</option>
          <option value="American">American</option>
          <option value="Dessert">Dessert</option>
        </select>
        <select
          value={typeFilter}
          onChange={(e) => setTypeFilter(e.target.value)}
          className="filter-select"
        >
          <option value="">All Types</option>
          <option value="Main Course">Main Course</option>
          <option value="Appetizer">Appetizer</option>
          <option value="Dessert">Dessert</option>
          <option value="Beverage">Beverage</option>
        </select>
        <button onClick={clearFilters} className="clear-button">Clear Filters</button>
      </div>

      <div className="recipes-grid">
        {recipes.map((recipe, index) => (
          <div key={index} className="recipe-card">
            <h3>{recipe.title}</h3>
            <div className="recipe-meta">
              <span className="category">{recipe.category}</span>
              <span className="type">{recipe.type}</span>
              <span className="cook-time">{recipe.cookTime}</span>
            </div>
            <div className="recipe-content">
              <p><strong>Ingredients:</strong> {recipe.ingredients}</p>
              <p><strong>Instructions:</strong> {recipe.instructions}</p>
              <p><strong>Serving Size:</strong> {recipe.servingSize}</p>
            </div>
            <div className="recipe-actions">
              <button onClick={() => onEdit(recipe)} className="edit-button">Edit</button>
              <button onClick={() => handleDelete(recipe.title)} className="delete-button">Delete</button>
            </div>
          </div>
        ))}
      </div>

      {recipes.length === 0 && (
        <div className="no-recipes">No recipes found. Try adjusting your filters or add a new recipe.</div>
      )}
    </div>
  );
};

export default RecipeList;
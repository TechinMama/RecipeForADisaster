import React, { useState, useEffect, useCallback } from 'react';
import { Recipe } from '../types/Recipe';
import { recipeApi } from '../services/api';
import AdvancedSearch from './AdvancedSearch';

interface RecipeListProps {
  onEdit: (recipe: Recipe) => void;
  onView: (recipe: Recipe) => void;
  refreshTrigger: number;
}

const RecipeList: React.FC<RecipeListProps> = ({ onEdit, onView, refreshTrigger }) => {
  // IndexedDB helpers for offline caching
  const OFFLINE_DB_NAME = 'RecipeOfflineDB';
  const OFFLINE_DB_VERSION = 1;
  const CACHED_RECIPES_STORE = 'cachedRecipes';

  function openOfflineDB(): Promise<IDBDatabase> {
    return new Promise((resolve, reject) => {
      const request = indexedDB.open(OFFLINE_DB_NAME, OFFLINE_DB_VERSION);
      request.onsuccess = () => resolve(request.result);
      request.onerror = () => reject(request.error);
    });
  }

  async function getCachedRecipes(): Promise<Recipe[]> {
    const db = await openOfflineDB();
    return new Promise((resolve, reject) => {
      const tx = db.transaction([CACHED_RECIPES_STORE], 'readonly');
      const store = tx.objectStore(CACHED_RECIPES_STORE);
      const req = store.getAll();
      req.onsuccess = () => resolve(req.result as Recipe[]);
      req.onerror = () => reject(req.error);
    });
  }
  const [recipes, setRecipes] = useState<Recipe[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [categoryFilter, setCategoryFilter] = useState('');
  const [typeFilter, setTypeFilter] = useState('');
  const [sortBy, setSortBy] = useState<'title' | 'category' | 'type' | 'cookTime'>('title');
  const [sortOrder, setSortOrder] = useState<'asc' | 'desc'>('asc');
  const [currentPage, setCurrentPage] = useState(1);
  const [useAdvancedSearch, setUseAdvancedSearch] = useState(false);
  const recipesPerPage = 6;

  const handleAdvancedSearchResults = (searchResults: Recipe[]) => {
    setRecipes(searchResults);
    setUseAdvancedSearch(searchResults.length > 0 || true); // Keep advanced mode active
    setCurrentPage(1);
  };

  const loadRecipes = useCallback(async () => {
    // Skip loading if using advanced search results
    if (useAdvancedSearch) return;

    setLoading(true);
    let data: Recipe[] = [];
    try {
      if (navigator.onLine) {
        if (searchQuery) {
          data = await recipeApi.searchRecipes(searchQuery);
        } else if (categoryFilter) {
          data = await recipeApi.getRecipesByCategory(categoryFilter);
        } else if (typeFilter) {
          data = await recipeApi.getRecipesByType(typeFilter);
        } else {
          data = await recipeApi.getAllRecipes();
        }
      } else {
        // Offline: try IndexedDB
        try {
          data = await getCachedRecipes();
        } catch (e) {
          setError('Offline and no cached recipes available');
        }
      }
      setRecipes(data);
      setError(null);
    } catch (err) {
      setError('Failed to load recipes');
      console.error('Error loading recipes:', err);
      setRecipes([]);
    } finally {
      setLoading(false);
    }
  }, [searchQuery, categoryFilter, typeFilter]);

  // Sort and paginate recipes
  const sortedRecipes = React.useMemo(() => {
    const sorted = [...recipes].sort((a, b) => {
      let aValue: string;
      let bValue: string;

      switch (sortBy) {
        case 'title':
          aValue = a.title.toLowerCase();
          bValue = b.title.toLowerCase();
          break;
        case 'category':
          aValue = a.category.toLowerCase();
          bValue = b.category.toLowerCase();
          break;
        case 'type':
          aValue = a.type.toLowerCase();
          bValue = b.type.toLowerCase();
          break;
        case 'cookTime':
          aValue = a.cookTime.toLowerCase();
          bValue = b.cookTime.toLowerCase();
          break;
        default:
          return 0;
      }

      if (sortOrder === 'asc') {
        return aValue.localeCompare(bValue);
      } else {
        return bValue.localeCompare(aValue);
      }
    });

    // Pagination
    const startIndex = (currentPage - 1) * recipesPerPage;
    const endIndex = startIndex + recipesPerPage;
    return sorted.slice(startIndex, endIndex);
  }, [recipes, sortBy, sortOrder, currentPage]);

  const totalPages = Math.ceil(recipes.length / recipesPerPage);

  useEffect(() => {
    loadRecipes();
  }, [loadRecipes, refreshTrigger]);

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
    setSortBy('title');
    setSortOrder('asc');
    setCurrentPage(1);
  };

  if (loading) return <div className="loading">Loading recipes...</div>;
  if (error) return <div className="error">{error}</div>;

  return (
    <div className="recipe-list">
      {/* Advanced Search Component */}
      <AdvancedSearch onResultsChange={handleAdvancedSearchResults} />

      {/* Basic filters (kept for quick access) */}
      <div className="filters">
        <input
          type="text"
          placeholder="Quick search recipes..."
          value={searchQuery}
          onChange={(e) => {
            setSearchQuery(e.target.value);
            setUseAdvancedSearch(false); // Switch to basic search
          }}
          className="search-input"
        />
        <select
          value={categoryFilter}
          onChange={(e) => {
            setCategoryFilter(e.target.value);
            setUseAdvancedSearch(false); // Switch to basic search
          }}
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
          onChange={(e) => {
            setTypeFilter(e.target.value);
            setUseAdvancedSearch(false); // Switch to basic search
          }}
          className="filter-select"
        >
          <option value="">All Types</option>
          <option value="Main Course">Main Course</option>
          <option value="Appetizer">Appetizer</option>
          <option value="Dessert">Dessert</option>
          <option value="Beverage">Beverage</option>
        </select>
        <select
          value={sortBy}
          onChange={(e) => setSortBy(e.target.value as 'title' | 'category' | 'type' | 'cookTime')}
          className="filter-select"
        >
          <option value="title">Sort by Title</option>
          <option value="category">Sort by Category</option>
          <option value="type">Sort by Type</option>
          <option value="cookTime">Sort by Cook Time</option>
        </select>
        <select
          value={sortOrder}
          onChange={(e) => setSortOrder(e.target.value as 'asc' | 'desc')}
          className="filter-select"
        >
          <option value="asc">Ascending</option>
          <option value="desc">Descending</option>
        </select>
        <button onClick={clearFilters} className="clear-button">Clear Filters</button>
      </div>

      <div className="recipes-grid">
        {sortedRecipes.map((recipe, index) => (
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
              <button onClick={() => onView(recipe)} className="view-button">View</button>
              <button onClick={() => onEdit(recipe)} className="edit-button">Edit</button>
              <button onClick={() => handleDelete(recipe.title)} className="delete-button">Delete</button>
            </div>
          </div>
        ))}
      </div>

      {recipes.length === 0 && (
        <div className="no-recipes">No recipes found. Try adjusting your filters or add a new recipe.</div>
      )}

      {totalPages > 1 && (
        <div className="pagination">
          <button
            onClick={() => setCurrentPage(prev => Math.max(prev - 1, 1))}
            disabled={currentPage === 1}
            className="page-button"
          >
            Previous
          </button>

          <span className="page-info">
            Page {currentPage} of {totalPages} ({recipes.length} total recipes)
          </span>

          <button
            onClick={() => setCurrentPage(prev => Math.min(prev + 1, totalPages))}
            disabled={currentPage === totalPages}
            className="page-button"
          >
            Next
          </button>
        </div>
      )}
    </div>
  );
};

export default RecipeList;
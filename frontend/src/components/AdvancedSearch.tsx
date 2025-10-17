import React, { useState, useEffect } from 'react';
import { Recipe } from '../types/Recipe';
import { advancedSearchRecipes } from '../services/api';
import './AdvancedSearch.css';

interface SearchCriteria {
  query: string;
  category: string;
  type: string;
  cookTimeMax: string;
  servingSizeMin: string;
  servingSizeMax: string;
  ingredient: string;
  sortBy: string;
  sortOrder: string;
}

interface AdvancedSearchProps {
  onResultsChange: (recipes: Recipe[]) => void;
}

const AdvancedSearch: React.FC<AdvancedSearchProps> = ({ onResultsChange }) => {
  const [isExpanded, setIsExpanded] = useState(false);
  const [isSearching, setIsSearching] = useState(false);
  const [criteria, setCriteria] = useState<SearchCriteria>({
    query: '',
    category: '',
    type: '',
    cookTimeMax: '',
    servingSizeMin: '',
    servingSizeMax: '',
    ingredient: '',
    sortBy: 'title',
    sortOrder: 'asc'
  });
  const [savedPresets, setSavedPresets] = useState<{ [key: string]: SearchCriteria }>({});
  const [presetName, setPresetName] = useState('');
  const [selectedPreset, setSelectedPreset] = useState('');

  // Load saved presets from localStorage
  useEffect(() => {
    const saved = localStorage.getItem('searchPresets');
    if (saved) {
      try {
        setSavedPresets(JSON.parse(saved));
      } catch (e) {
        console.error('Failed to load search presets:', e);
      }
    }
  }, []);

  // Update URL with search criteria
  useEffect(() => {
    const params = new URLSearchParams();
    Object.entries(criteria).forEach(([key, value]) => {
      if (value) params.set(key, value);
    });
    const newUrl = params.toString() ? `?${params.toString()}` : window.location.pathname;
    window.history.replaceState({}, '', newUrl);
  }, [criteria]);

  // Load criteria from URL on mount
  useEffect(() => {
    const params = new URLSearchParams(window.location.search);
    const urlCriteria: Partial<SearchCriteria> = {};
    params.forEach((value, key) => {
      if (key in criteria) {
        urlCriteria[key as keyof SearchCriteria] = value;
      }
    });
    if (Object.keys(urlCriteria).length > 0) {
      setCriteria({ ...criteria, ...urlCriteria });
      setIsExpanded(true);
    }
  }, []);

  const handleInputChange = (field: keyof SearchCriteria, value: string) => {
    setCriteria({ ...criteria, [field]: value });
  };

  const handleSearch = async () => {
    setIsSearching(true);
    try {
      const results = await advancedSearchRecipes(criteria);
      onResultsChange(results);
    } catch (error) {
      console.error('Search failed:', error);
      alert('Search failed. Please try again.');
    } finally {
      setIsSearching(false);
    }
  };

  const handleClearFilters = () => {
    setCriteria({
      query: '',
      category: '',
      type: '',
      cookTimeMax: '',
      servingSizeMin: '',
      servingSizeMax: '',
      ingredient: '',
      sortBy: 'title',
      sortOrder: 'asc'
    });
    onResultsChange([]);
  };

  const handleSavePreset = () => {
    if (!presetName.trim()) {
      alert('Please enter a name for this preset');
      return;
    }
    const newPresets = { ...savedPresets, [presetName]: criteria };
    setSavedPresets(newPresets);
    localStorage.setItem('searchPresets', JSON.stringify(newPresets));
    setPresetName('');
    alert(`Preset "${presetName}" saved!`);
  };

  const handleLoadPreset = (name: string) => {
    if (savedPresets[name]) {
      setCriteria(savedPresets[name]);
      setSelectedPreset(name);
      handleSearch();
    }
  };

  const handleDeletePreset = (name: string) => {
    const newPresets = { ...savedPresets };
    delete newPresets[name];
    setSavedPresets(newPresets);
    localStorage.setItem('searchPresets', JSON.stringify(newPresets));
    if (selectedPreset === name) {
      setSelectedPreset('');
    }
  };

  return (
    <div className="advanced-search">
      <div className="search-header">
        <h2>
          <span className="search-icon">🔍</span>
          Advanced Recipe Search
        </h2>
        <button 
          className="toggle-button"
          onClick={() => setIsExpanded(!isExpanded)}
          aria-label={isExpanded ? "Collapse filters" : "Expand filters"}
        >
          {isExpanded ? '▲ Hide Filters' : '▼ Show Filters'}
        </button>
      </div>

      {isExpanded && (
        <div className="search-content">
          {/* Full-text search */}
          <div className="search-section">
            <label htmlFor="query">Search All Fields:</label>
            <input
              id="query"
              type="text"
              placeholder="Search recipes, ingredients, instructions..."
              value={criteria.query}
              onChange={(e) => handleInputChange('query', e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && handleSearch()}
            />
          </div>

          {/* Filter grid */}
          <div className="filters-grid">
            <div className="filter-group">
              <label htmlFor="category">Category:</label>
              <select
                id="category"
                value={criteria.category}
                onChange={(e) => handleInputChange('category', e.target.value)}
              >
                <option value="">All Categories</option>
                <option value="Italian">Italian</option>
                <option value="Mexican">Mexican</option>
                <option value="Asian">Asian</option>
                <option value="American">American</option>
                <option value="Mediterranean">Mediterranean</option>
                <option value="Indian">Indian</option>
                <option value="French">French</option>
                <option value="Thai">Thai</option>
                <option value="Other">Other</option>
              </select>
            </div>

            <div className="filter-group">
              <label htmlFor="type">Meal Type:</label>
              <select
                id="type"
                value={criteria.type}
                onChange={(e) => handleInputChange('type', e.target.value)}
              >
                <option value="">All Types</option>
                <option value="Breakfast">Breakfast</option>
                <option value="Lunch">Lunch</option>
                <option value="Dinner">Dinner</option>
                <option value="Appetizer">Appetizer</option>
                <option value="Dessert">Dessert</option>
                <option value="Snack">Snack</option>
                <option value="Beverage">Beverage</option>
              </select>
            </div>

            <div className="filter-group">
              <label htmlFor="ingredient">Contains Ingredient:</label>
              <input
                id="ingredient"
                type="text"
                placeholder="e.g., chicken, tomato"
                value={criteria.ingredient}
                onChange={(e) => handleInputChange('ingredient', e.target.value)}
              />
            </div>

            <div className="filter-group">
              <label htmlFor="cookTimeMax">Max Cook Time (minutes):</label>
              <input
                id="cookTimeMax"
                type="number"
                min="0"
                placeholder="e.g., 30"
                value={criteria.cookTimeMax}
                onChange={(e) => handleInputChange('cookTimeMax', e.target.value)}
              />
            </div>

            <div className="filter-group">
              <label htmlFor="servingSizeMin">Min Servings:</label>
              <input
                id="servingSizeMin"
                type="number"
                min="1"
                placeholder="e.g., 2"
                value={criteria.servingSizeMin}
                onChange={(e) => handleInputChange('servingSizeMin', e.target.value)}
              />
            </div>

            <div className="filter-group">
              <label htmlFor="servingSizeMax">Max Servings:</label>
              <input
                id="servingSizeMax"
                type="number"
                min="1"
                placeholder="e.g., 8"
                value={criteria.servingSizeMax}
                onChange={(e) => handleInputChange('servingSizeMax', e.target.value)}
              />
            </div>

            <div className="filter-group">
              <label htmlFor="sortBy">Sort By:</label>
              <select
                id="sortBy"
                value={criteria.sortBy}
                onChange={(e) => handleInputChange('sortBy', e.target.value)}
              >
                <option value="title">Title</option>
                <option value="cookTime">Cook Time</option>
                <option value="category">Category</option>
              </select>
            </div>

            <div className="filter-group">
              <label htmlFor="sortOrder">Sort Order:</label>
              <select
                id="sortOrder"
                value={criteria.sortOrder}
                onChange={(e) => handleInputChange('sortOrder', e.target.value)}
              >
                <option value="asc">Ascending</option>
                <option value="desc">Descending</option>
              </select>
            </div>
          </div>

          {/* Action buttons */}
          <div className="search-actions">
            <button 
              className="btn-primary"
              onClick={handleSearch}
              disabled={isSearching}
            >
              {isSearching ? '🔍 Searching...' : '🔍 Search'}
            </button>
            <button 
              className="btn-secondary"
              onClick={handleClearFilters}
            >
              ✖ Clear Filters
            </button>
          </div>

          {/* Presets section */}
          <div className="presets-section">
            <h3>Search Presets</h3>
            <div className="preset-save">
              <input
                type="text"
                placeholder="Preset name..."
                value={presetName}
                onChange={(e) => setPresetName(e.target.value)}
              />
              <button 
                className="btn-small"
                onClick={handleSavePreset}
              >
                💾 Save Current Search
              </button>
            </div>
            
            {Object.keys(savedPresets).length > 0 && (
              <div className="preset-list">
                <h4>Saved Presets:</h4>
                {Object.keys(savedPresets).map((name) => (
                  <div key={name} className="preset-item">
                    <button
                      className={`preset-load ${selectedPreset === name ? 'active' : ''}`}
                      onClick={() => handleLoadPreset(name)}
                    >
                      📋 {name}
                    </button>
                    <button
                      className="preset-delete"
                      onClick={() => handleDeletePreset(name)}
                      title="Delete preset"
                    >
                      🗑️
                    </button>
                  </div>
                ))}
              </div>
            )}
          </div>
        </div>
      )}
    </div>
  );
};

export default AdvancedSearch;

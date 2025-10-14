import React, { useState } from 'react';
import RecipeList from './components/RecipeList';
import RecipeForm from './components/RecipeForm';
import { Recipe } from './types/Recipe';
import './App.css';

function App() {
  const [currentView, setCurrentView] = useState<'list' | 'form'>('list');
  const [editingRecipe, setEditingRecipe] = useState<Recipe | undefined>();
  const [refreshTrigger, setRefreshTrigger] = useState(0);

  const handleAddRecipe = () => {
    setEditingRecipe(undefined);
    setCurrentView('form');
  };

  const handleEditRecipe = (recipe: Recipe) => {
    setEditingRecipe(recipe);
    setCurrentView('form');
  };

  const handleSaveRecipe = () => {
    setCurrentView('list');
    setRefreshTrigger(prev => prev + 1); // Trigger refresh of recipe list
  };

  const handleCancelForm = () => {
    setCurrentView('list');
    setEditingRecipe(undefined);
  };

  return (
    <div className="App">
      <header className="App-header">
        <h1>RecipeForADisaster</h1>
        <p>AI-Powered Recipe Manager</p>
      </header>

      <main className="App-main">
        {currentView === 'list' ? (
          <div className="list-view">
            <div className="actions-bar">
              <button onClick={handleAddRecipe} className="add-recipe-button">
                Add New Recipe
              </button>
            </div>
            <RecipeList
              onEdit={handleEditRecipe}
              refreshTrigger={refreshTrigger}
            />
          </div>
        ) : (
          <RecipeForm
            recipe={editingRecipe}
            onSave={handleSaveRecipe}
            onCancel={handleCancelForm}
          />
        )}
      </main>

      <footer className="App-footer">
        <p>&copy; 2025 RecipeForADisaster - Powered by AI and Crow</p>
      </footer>
    </div>
  );
}

export default App;

import React, { useState } from 'react';
import { Recipe } from '../types/Recipe';

interface RecipePreviewProps {
  recipe: Partial<Recipe>;
  onSave: (recipe: Recipe) => void;
  onEdit: () => void;
  onCancel: () => void;
  isLoading?: boolean;
}

const RecipePreview: React.FC<RecipePreviewProps> = ({
  recipe,
  onSave,
  onEdit,
  onCancel,
  isLoading = false
}) => {
  const [editedRecipe, setEditedRecipe] = useState<Partial<Recipe>>(recipe);
  const [isEditing, setIsEditing] = useState(false);

  const handleFieldChange = (field: keyof Recipe, value: string) => {
    setEditedRecipe(prev => ({
      ...prev,
      [field]: value
    }));
  };

  const handleIngredientsChange = (value: string) => {
    setEditedRecipe(prev => ({
      ...prev,
      ingredients: value
    }));
  };

  const handleSave = () => {
    if (editedRecipe.title && editedRecipe.ingredients && editedRecipe.instructions) {
      onSave(editedRecipe as Recipe);
    }
  };

  const toggleEdit = () => {
    setIsEditing(!isEditing);
    if (!isEditing) {
      setEditedRecipe(recipe); // Reset to original when starting edit
    }
  };

  if (isLoading) {
    return (
      <div className="recipe-preview-loading">
        <div className="loading-spinner"></div>
        <p>Preparing your recipe...</p>
      </div>
    );
  }

  return (
    <div className="recipe-preview">
      <div className="recipe-preview-header">
        <h2>🎉 Recipe Generated!</h2>
        <p>Review and customize your AI-generated recipe before saving</p>
      </div>

      <div className="recipe-preview-content">
        <div className="recipe-card">
          <div className="recipe-header">
            {isEditing ? (
              <input
                type="text"
                value={editedRecipe.title || ''}
                onChange={(e) => handleFieldChange('title', e.target.value)}
                placeholder="Recipe Title"
                className="edit-input title-input"
              />
            ) : (
              <h3>{editedRecipe.title}</h3>
            )}
          </div>

          <div className="recipe-meta">
            <div className="meta-row">
              <div className="meta-item">
                <label>Category:</label>
                {isEditing ? (
                  <select
                    value={editedRecipe.category || ''}
                    onChange={(e) => handleFieldChange('category', e.target.value)}
                  >
                    <option value="">Select Category</option>
                    <option value="Appetizer">Appetizer</option>
                    <option value="Main Course">Main Course</option>
                    <option value="Side Dish">Side Dish</option>
                    <option value="Dessert">Dessert</option>
                    <option value="Beverage">Beverage</option>
                    <option value="Breakfast">Breakfast</option>
                    <option value="Lunch">Lunch</option>
                    <option value="Dinner">Dinner</option>
                  </select>
                ) : (
                  <span>{editedRecipe.category || 'Not specified'}</span>
                )}
              </div>

              <div className="meta-item">
                <label>Type:</label>
                {isEditing ? (
                  <input
                    type="text"
                    value={editedRecipe.type || ''}
                    onChange={(e) => handleFieldChange('type', e.target.value)}
                    placeholder="e.g., Pasta, Salad, Soup"
                  />
                ) : (
                  <span>{editedRecipe.type || 'Not specified'}</span>
                )}
              </div>
            </div>

            <div className="meta-row">
              <div className="meta-item">
                <label>Serving Size:</label>
                {isEditing ? (
                  <input
                    type="text"
                    value={editedRecipe.servingSize || ''}
                    onChange={(e) => handleFieldChange('servingSize', e.target.value)}
                    placeholder="e.g., 4 servings"
                  />
                ) : (
                  <span>{editedRecipe.servingSize || 'Not specified'}</span>
                )}
              </div>

              <div className="meta-item">
                <label>Cook Time:</label>
                {isEditing ? (
                  <input
                    type="text"
                    value={editedRecipe.cookTime || ''}
                    onChange={(e) => handleFieldChange('cookTime', e.target.value)}
                    placeholder="e.g., 30 minutes"
                  />
                ) : (
                  <span>{editedRecipe.cookTime || 'Not specified'}</span>
                )}
              </div>
            </div>
          </div>

          <div className="recipe-section">
            <h4>Ingredients</h4>
            {isEditing ? (
              <textarea
                value={editedRecipe.ingredients || ''}
                onChange={(e) => handleIngredientsChange(e.target.value)}
                placeholder="List your ingredients, one per line..."
                rows={6}
              />
            ) : (
              <div className="ingredients-list">
                {editedRecipe.ingredients?.split('\n').map((ingredient, index) => (
                  ingredient.trim() && (
                    <div key={index} className="ingredient-item">
                      • {ingredient.trim()}
                    </div>
                  )
                )) || <p>No ingredients specified</p>}
              </div>
            )}
          </div>

          <div className="recipe-section">
            <h4>Instructions</h4>
            {isEditing ? (
              <textarea
                value={editedRecipe.instructions || ''}
                onChange={(e) => handleFieldChange('instructions', e.target.value)}
                placeholder="Describe the cooking steps..."
                rows={8}
              />
            ) : (
              <div className="instructions-content">
                {editedRecipe.instructions?.split('\n').map((instruction, index) => (
                  instruction.trim() && (
                    <div key={index} className="instruction-step">
                      <span className="step-number">{index + 1}.</span> {instruction.trim()}
                    </div>
                  )
                )) || <p>No instructions specified</p>}
              </div>
            )}
          </div>
        </div>
      </div>

      <div className="recipe-preview-actions">
        <button onClick={toggleEdit} className="edit-button">
          {isEditing ? 'Cancel Edit' : '✏️ Edit Recipe'}
        </button>

        <div className="action-buttons">
          <button onClick={onCancel} className="cancel-button">
            Generate Another
          </button>
          <button
            onClick={handleSave}
            className="save-button"
            disabled={!editedRecipe.title || !editedRecipe.ingredients || !editedRecipe.instructions}
          >
            💾 Save Recipe
          </button>
        </div>
      </div>
    </div>
  );
};

export default RecipePreview;
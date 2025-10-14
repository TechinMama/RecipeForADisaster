import React from 'react';
import { Recipe } from '../types/Recipe';

interface RecipeDetailsProps {
  recipe: Recipe;
  onClose: () => void;
  onEdit: () => void;
}

const RecipeDetails: React.FC<RecipeDetailsProps> = ({ recipe, onClose, onEdit }) => {
  return (
    <div className="recipe-details-overlay" onClick={onClose}>
      <div className="recipe-details-modal" onClick={(e) => e.stopPropagation()}>
        <div className="recipe-details-header">
          <h2>{recipe.title}</h2>
          <button onClick={onClose} className="close-button">×</button>
        </div>

        <div className="recipe-details-content">
          <div className="recipe-details-meta">
            <div className="meta-item">
              <strong>Category:</strong> {recipe.category}
            </div>
            <div className="meta-item">
              <strong>Type:</strong> {recipe.type}
            </div>
            <div className="meta-item">
              <strong>Serving Size:</strong> {recipe.servingSize}
            </div>
            <div className="meta-item">
              <strong>Cook Time:</strong> {recipe.cookTime}
            </div>
          </div>

          <div className="recipe-details-section">
            <h3>Ingredients</h3>
            <div className="ingredients-list">
              {recipe.ingredients.split('\n').map((ingredient, index) => (
                <div key={index} className="ingredient-item">
                  • {ingredient.trim()}
                </div>
              ))}
            </div>
          </div>

          <div className="recipe-details-section">
            <h3>Instructions</h3>
            <div className="instructions-content">
              {recipe.instructions.split('\n').map((instruction, index) => (
                <div key={index} className="instruction-step">
                  <span className="step-number">{index + 1}.</span>
                  <span className="step-text">{instruction.trim()}</span>
                </div>
              ))}
            </div>
          </div>
        </div>

        <div className="recipe-details-actions">
          <button onClick={onEdit} className="edit-button">Edit Recipe</button>
          <button onClick={onClose} className="close-button-secondary">Close</button>
        </div>
      </div>
    </div>
  );
};

export default RecipeDetails;
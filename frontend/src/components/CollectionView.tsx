import React, { useState, useEffect } from 'react';
import { Collection, Recipe } from '../types/Recipe';
import { getCollection, getAllRecipes, addRecipeToCollection, removeRecipeFromCollection } from '../services/api';

interface CollectionViewProps {
  collection: Collection;
  onClose: () => void;
  onEdit: () => void;
}

const CollectionView: React.FC<CollectionViewProps> = ({ collection: initialCollection, onClose, onEdit }) => {
  const [collection, setCollection] = useState<Collection>(initialCollection);
  const [allRecipes, setAllRecipes] = useState<Recipe[]>([]);
  const [collectionRecipes, setCollectionRecipes] = useState<Recipe[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [showAddRecipe, setShowAddRecipe] = useState(false);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [collectionData, recipesData] = await Promise.all([
          getCollection(initialCollection.id),
          getAllRecipes()
        ]);

        setCollection(collectionData);
        setAllRecipes(recipesData);

        // For now, we'll assume collection doesn't have recipes list
        // In a real implementation, you'd fetch recipes in collection
        setCollectionRecipes([]);
      } catch (err) {
        setError('Failed to load collection details');
        console.error('Error fetching collection data:', err);
      } finally {
        setLoading(false);
      }
    };

    fetchData();
  }, [initialCollection.id]);

  const handleAddRecipe = async (recipe: Recipe) => {
    try {
      await addRecipeToCollection(collection.id, recipe.title);
      setCollectionRecipes(prev => [...prev, recipe]);
      setShowAddRecipe(false);
    } catch (err) {
      setError('Failed to add recipe to collection');
      console.error('Error adding recipe to collection:', err);
    }
  };

  const handleRemoveRecipe = async (recipe: Recipe) => {
    try {
      await removeRecipeFromCollection(collection.id, recipe.title);
      setCollectionRecipes(prev => prev.filter(r => r.title !== recipe.title));
    } catch (err) {
      setError('Failed to remove recipe from collection');
      console.error('Error removing recipe from collection:', err);
    }
  };

  const availableRecipes = allRecipes.filter(recipe =>
    !collectionRecipes.some(cr => cr.title === recipe.title)
  );

  if (loading) {
    return (
      <div className="collection-overlay" onClick={onClose}>
        <div className="collection-modal" onClick={(e) => e.stopPropagation()}>
          <div className="collection-loading">
            <p>Loading collection...</p>
          </div>
        </div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="collection-overlay" onClick={onClose}>
        <div className="collection-modal" onClick={(e) => e.stopPropagation()}>
          <div className="collection-error">
            <p>{error}</p>
            <button onClick={onClose} className="close-button-secondary">Close</button>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="collection-overlay" onClick={onClose}>
      <div className="collection-modal" onClick={(e) => e.stopPropagation()}>
        <div className="collection-header">
          <h2>{collection.name}</h2>
          <button onClick={onClose} className="close-button">×</button>
        </div>

        <div className="collection-content">
          <div className="collection-info">
            <p className="collection-description">{collection.description}</p>
            <div className="collection-meta">
              <span>Created: {new Date(collection.createdAt).toLocaleDateString()}</span>
              <span>Last updated: {new Date(collection.updatedAt).toLocaleDateString()}</span>
            </div>
          </div>

          <div className="collection-recipes-section">
            <div className="collection-recipes-header">
              <h3>Recipes in this Collection ({collectionRecipes.length})</h3>
              <button
                onClick={() => setShowAddRecipe(!showAddRecipe)}
                className="add-recipe-button"
              >
                {showAddRecipe ? 'Cancel' : 'Add Recipe'}
              </button>
            </div>

            {showAddRecipe && (
              <div className="add-recipe-section">
                <h4>Available Recipes</h4>
                {availableRecipes.length === 0 ? (
                  <p>No more recipes available to add.</p>
                ) : (
                  <div className="available-recipes-list">
                    {availableRecipes.map((recipe) => (
                      <div key={recipe.title} className="available-recipe-item">
                        <span>{recipe.title}</span>
                        <button
                          onClick={() => handleAddRecipe(recipe)}
                          className="add-to-collection-button"
                        >
                          Add
                        </button>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            )}

            <div className="collection-recipes-list">
              {collectionRecipes.length === 0 ? (
                <p className="no-recipes">No recipes in this collection yet.</p>
              ) : (
                collectionRecipes.map((recipe) => (
                  <div key={recipe.title} className="collection-recipe-item">
                    <div className="recipe-info">
                      <h4>{recipe.title}</h4>
                      <p>{recipe.category} • {recipe.type} • {recipe.cookTime}</p>
                    </div>
                    <button
                      onClick={() => handleRemoveRecipe(recipe)}
                      className="remove-recipe-button"
                      title="Remove from collection"
                    >
                      Remove
                    </button>
                  </div>
                ))
              )}
            </div>
          </div>
        </div>

        <div className="collection-actions">
          <button onClick={onEdit} className="edit-button">Edit Collection</button>
          <button onClick={onClose} className="close-button-secondary">Close</button>
        </div>
      </div>
    </div>
  );
};

export default CollectionView;
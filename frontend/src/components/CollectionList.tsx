import React, { useState, useEffect } from 'react';
import { Collection } from '../types/Recipe';
import { getCollections, deleteCollection } from '../services/api';

interface CollectionListProps {
  onViewCollection: (collection: Collection) => void;
  onCreateCollection: () => void;
}

const CollectionList: React.FC<CollectionListProps> = ({ onViewCollection, onCreateCollection }) => {
  const [collections, setCollections] = useState<Collection[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchCollections = async () => {
    try {
      const collectionsData = await getCollections();
      setCollections(collectionsData);
    } catch (err) {
      setError('Failed to load collections');
      console.error('Error fetching collections:', err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchCollections();
  }, []);

  const handleDeleteCollection = async (collectionId: string, e: React.MouseEvent) => {
    e.stopPropagation();
    if (!window.confirm('Are you sure you want to delete this collection?')) {
      return;
    }

    try {
      await deleteCollection(collectionId);
      setCollections(prev => prev.filter(c => c.id !== collectionId));
    } catch (err) {
      setError('Failed to delete collection');
      console.error('Error deleting collection:', err);
    }
  };

  if (loading) {
    return (
      <div className="collection-list">
        <div className="collection-loading">
          <p>Loading collections...</p>
        </div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="collection-list">
        <div className="collection-error">
          <p>{error}</p>
          <button onClick={fetchCollections} className="retry-button">Retry</button>
        </div>
      </div>
    );
  }

  return (
    <div className="collection-list">
      <div className="collection-header">
        <h2>My Collections</h2>
        <button onClick={onCreateCollection} className="create-collection-button">
          Create Collection
        </button>
      </div>

      {collections.length === 0 ? (
        <div className="no-collections">
          <p>You haven't created any collections yet.</p>
          <button onClick={onCreateCollection} className="create-first-collection-button">
            Create Your First Collection
          </button>
        </div>
      ) : (
        <div className="collections-grid">
          {collections.map((collection) => (
            <div
              key={collection.id}
              className="collection-card"
              onClick={() => onViewCollection(collection)}
            >
              <div className="collection-card-header">
                <h3>{collection.name}</h3>
                <button
                  onClick={(e) => handleDeleteCollection(collection.id, e)}
                  className="delete-collection-button"
                  title="Delete collection"
                >
                  ×
                </button>
              </div>

              <div className="collection-card-content">
                <p className="collection-description">
                  {collection.description || 'No description'}
                </p>
                <div className="collection-meta">
                  <span className="collection-date">
                    Created: {new Date(collection.createdAt).toLocaleDateString()}
                  </span>
                </div>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
};

export default CollectionList;
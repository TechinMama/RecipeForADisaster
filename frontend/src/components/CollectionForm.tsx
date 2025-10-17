import React, { useState } from 'react';
import { Collection } from '../types/Recipe';
import { createCollection, updateCollection } from '../services/api';

interface CollectionFormProps {
  collection?: Collection;
  onSave: (collection: Collection) => void;
  onCancel: () => void;
}

const CollectionForm: React.FC<CollectionFormProps> = ({ collection, onSave, onCancel }) => {
  const [formData, setFormData] = useState({
    name: collection?.name || '',
    description: collection?.description || '',
    privacySettings: collection?.privacySettings || {}
  });
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement | HTMLTextAreaElement>) => {
    const { name, value } = e.target;
    setFormData(prev => ({
      ...prev,
      [name]: value
    }));
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();

    if (!formData.name.trim()) {
      setError('Collection name is required');
      return;
    }

    setSaving(true);
    setError(null);

    try {
      let result;
      if (collection) {
        // Update existing collection
        result = await updateCollection(collection.id, formData);
        onSave({ ...collection, ...formData });
      } else {
        // Create new collection
        result = await createCollection(formData);
        onSave(result.data);
      }
    } catch (err) {
      setError(collection ? 'Failed to update collection' : 'Failed to create collection');
      console.error('Error saving collection:', err);
    } finally {
      setSaving(false);
    }
  };

  return (
    <div className="collection-overlay">
      <div className="collection-modal" onClick={(e) => e.stopPropagation()}>
        <div className="collection-header">
          <h2>{collection ? 'Edit Collection' : 'Create New Collection'}</h2>
          <button onClick={onCancel} className="close-button">×</button>
        </div>

        <form onSubmit={handleSubmit} className="collection-form">
          <div className="form-group">
            <label htmlFor="name">Collection Name *</label>
            <input
              type="text"
              id="name"
              name="name"
              value={formData.name}
              onChange={handleInputChange}
              placeholder="Enter collection name"
              required
            />
          </div>

          <div className="form-group">
            <label htmlFor="description">Description</label>
            <textarea
              id="description"
              name="description"
              value={formData.description}
              onChange={handleInputChange}
              placeholder="Describe your collection"
              rows={4}
            />
          </div>

          {error && (
            <div className="error-message">
              <p>{error}</p>
            </div>
          )}

          <div className="collection-actions">
            <button type="submit" disabled={saving} className="save-button">
              {saving ? 'Saving...' : (collection ? 'Update Collection' : 'Create Collection')}
            </button>
            <button type="button" onClick={onCancel} className="cancel-button">
              Cancel
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};

export default CollectionForm;
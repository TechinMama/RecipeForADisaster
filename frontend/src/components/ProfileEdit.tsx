import React, { useState, useEffect } from 'react';
import { User } from '../types/Recipe';
import { getCurrentUser, updateCurrentUser } from '../services/api';

interface ProfileEditProps {
  onSave: () => void;
  onCancel: () => void;
}

const ProfileEdit: React.FC<ProfileEditProps> = ({ onSave, onCancel }) => {
  const [user, setUser] = useState<User | null>(null);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [formData, setFormData] = useState({
    name: '',
    bio: '',
    avatarUrl: ''
  });

  useEffect(() => {
    const fetchUser = async () => {
      try {
        const userData = await getCurrentUser();
        setUser(userData);
        setFormData({
          name: userData.name || '',
          bio: userData.bio || '',
          avatarUrl: userData.avatarUrl || ''
        });
      } catch (err) {
        setError('Failed to load profile');
        console.error('Error fetching user profile:', err);
      } finally {
        setLoading(false);
      }
    };

    fetchUser();
  }, []);

  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement | HTMLTextAreaElement>) => {
    const { name, value } = e.target;
    setFormData(prev => ({
      ...prev,
      [name]: value
    }));
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!user) return;

    setSaving(true);
    setError(null);

    try {
      await updateCurrentUser(formData);
      onSave();
    } catch (err) {
      setError('Failed to update profile');
      console.error('Error updating profile:', err);
    } finally {
      setSaving(false);
    }
  };

  if (loading) {
    return (
      <div className="profile-overlay">
        <div className="profile-modal">
          <div className="profile-loading">
            <p>Loading profile...</p>
          </div>
        </div>
      </div>
    );
  }

  if (error || !user) {
    return (
      <div className="profile-overlay">
        <div className="profile-modal">
          <div className="profile-header">
            <h2>Profile Error</h2>
            <button onClick={onCancel} className="close-button">×</button>
          </div>
          <div className="profile-error">
            <p>{error || 'Profile not found'}</p>
            <button onClick={onCancel} className="close-button-secondary">Close</button>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="profile-overlay">
      <div className="profile-modal" onClick={(e) => e.stopPropagation()}>
        <div className="profile-header">
          <h2>Edit Profile</h2>
          <button onClick={onCancel} className="close-button">×</button>
        </div>

        <form onSubmit={handleSubmit} className="profile-edit-form">
          <div className="form-group">
            <label htmlFor="name">Name:</label>
            <input
              type="text"
              id="name"
              name="name"
              value={formData.name}
              onChange={handleInputChange}
              placeholder="Enter your name"
            />
          </div>

          <div className="form-group">
            <label htmlFor="bio">Bio:</label>
            <textarea
              id="bio"
              name="bio"
              value={formData.bio}
              onChange={handleInputChange}
              placeholder="Tell us about yourself"
              rows={4}
            />
          </div>

          <div className="form-group">
            <label htmlFor="avatarUrl">Avatar URL:</label>
            <input
              type="url"
              id="avatarUrl"
              name="avatarUrl"
              value={formData.avatarUrl}
              onChange={handleInputChange}
              placeholder="https://example.com/avatar.jpg"
            />
          </div>

          {error && (
            <div className="error-message">
              <p>{error}</p>
            </div>
          )}

          <div className="profile-actions">
            <button type="submit" disabled={saving} className="save-button">
              {saving ? 'Saving...' : 'Save Changes'}
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

export default ProfileEdit;
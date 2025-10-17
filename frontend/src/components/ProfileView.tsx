import React, { useState, useEffect } from 'react';
import { User } from '../types/Recipe';
import { getCurrentUser } from '../services/api';

interface ProfileViewProps {
  onEdit: () => void;
  onClose: () => void;
}

const ProfileView: React.FC<ProfileViewProps> = ({ onEdit, onClose }) => {
  const [user, setUser] = useState<User | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const fetchUser = async () => {
      try {
        const userData = await getCurrentUser();
        setUser(userData);
      } catch (err) {
        setError('Failed to load profile');
        console.error('Error fetching user profile:', err);
      } finally {
        setLoading(false);
      }
    };

    fetchUser();
  }, []);

  if (loading) {
    return (
      <div className="profile-overlay" onClick={onClose}>
        <div className="profile-modal" onClick={(e) => e.stopPropagation()}>
          <div className="profile-loading">
            <p>Loading profile...</p>
          </div>
        </div>
      </div>
    );
  }

  if (error || !user) {
    return (
      <div className="profile-overlay" onClick={onClose}>
        <div className="profile-modal" onClick={(e) => e.stopPropagation()}>
          <div className="profile-header">
            <h2>Profile Error</h2>
            <button onClick={onClose} className="close-button">×</button>
          </div>
          <div className="profile-error">
            <p>{error || 'Profile not found'}</p>
            <button onClick={onClose} className="close-button-secondary">Close</button>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="profile-overlay" onClick={onClose}>
      <div className="profile-modal" onClick={(e) => e.stopPropagation()}>
        <div className="profile-header">
          <h2>My Profile</h2>
          <button onClick={onClose} className="close-button">×</button>
        </div>

        <div className="profile-content">
          <div className="profile-avatar-section">
            {user.avatarUrl ? (
              <img src={user.avatarUrl} alt="Profile" className="profile-avatar" />
            ) : (
              <div className="profile-avatar-placeholder">
                {user.name ? user.name.charAt(0).toUpperCase() : user.email.charAt(0).toUpperCase()}
              </div>
            )}
          </div>

          <div className="profile-info-section">
            <div className="profile-field">
              <label>Name:</label>
              <p>{user.name || 'Not set'}</p>
            </div>

            <div className="profile-field">
              <label>Email:</label>
              <p>{user.email}</p>
            </div>

            <div className="profile-field">
              <label>Bio:</label>
              <p>{user.bio || 'No bio yet'}</p>
            </div>

            <div className="profile-field">
              <label>Member since:</label>
              <p>{new Date(user.createdAt).toLocaleDateString()}</p>
            </div>

            <div className="profile-field">
              <label>Last updated:</label>
              <p>{new Date(user.updatedAt).toLocaleDateString()}</p>
            </div>
          </div>
        </div>

        <div className="profile-actions">
          <button onClick={onEdit} className="edit-button">Edit Profile</button>
          <button onClick={onClose} className="close-button-secondary">Close</button>
        </div>
      </div>
    </div>
  );
};

export default ProfileView;
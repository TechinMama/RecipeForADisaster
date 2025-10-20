
import React, { useState, Suspense } from 'react';
import RecipeForm from './components/RecipeForm';
import AIGeneration from './components/AIGeneration';
import ProfileView from './components/ProfileView';
import ProfileEdit from './components/ProfileEdit';
import CollectionList from './components/CollectionList';
import CollectionView from './components/CollectionView';
import CollectionForm from './components/CollectionForm';
import Login from './components/Login';
import Register from './components/Register';
import PWAInstallPrompt from './components/PWAInstallPrompt';
import OfflineIndicator from './components/OfflineIndicator';
import { AuthProvider, useAuth } from './contexts/AuthContext';
import { Recipe, Collection, User } from './types/Recipe';
import './App.css';

const RecipeList = React.lazy(() => import('./components/RecipeList'));
const RecipeDetails = React.lazy(() => import('./components/RecipeDetails'));

function AppContent() {
  const { user, login, logout, isAuthenticated, loading } = useAuth();
  const [currentView, setCurrentView] = useState<'list' | 'form' | 'details' | 'ai' | 'profile' | 'profile-edit' | 'collections' | 'collection-view' | 'collection-form'>('list');
  const [editingRecipe, setEditingRecipe] = useState<Recipe | undefined>();
  const [viewingRecipe, setViewingRecipe] = useState<Recipe | undefined>();
  const [viewingCollection, setViewingCollection] = useState<Collection | undefined>();
  const [editingCollection, setEditingCollection] = useState<Collection | undefined>();
  const [refreshTrigger, setRefreshTrigger] = useState(0);
  const [showAuth, setShowAuth] = useState<'login' | 'register' | null>(null);

  const handleAddRecipe = () => {
    setEditingRecipe(undefined);
    setCurrentView('form');
  };

  const handleAIGeneration = () => {
    setCurrentView('ai');
  };

  const handleEditRecipe = (recipe: Recipe) => {
    setEditingRecipe(recipe);
    setCurrentView('form');
  };

  const handleViewRecipe = (recipe: Recipe) => {
    setViewingRecipe(recipe);
    setCurrentView('details');
  };

  const handleSaveRecipe = () => {
    setCurrentView('list');
    setRefreshTrigger(prev => prev + 1); // Trigger refresh of recipe list
  };

  const handleCancelForm = () => {
    setCurrentView('list');
    setEditingRecipe(undefined);
  };

  const handleCloseDetails = () => {
    setCurrentView('list');
    setViewingRecipe(undefined);
  };

  const handleEditFromDetails = () => {
    if (viewingRecipe) {
      setEditingRecipe(viewingRecipe);
      setViewingRecipe(undefined);
      setCurrentView('form');
    }
  };

  const handleViewProfile = () => {
    setCurrentView('profile');
  };

  const handleEditProfile = () => {
    setCurrentView('profile-edit');
  };

  const handleSaveProfile = () => {
    setCurrentView('profile');
  };

  const handleCancelProfileEdit = () => {
    setCurrentView('profile');
  };

  const handleViewCollections = () => {
    setCurrentView('collections');
  };

  const handleViewCollection = (collection: Collection) => {
    setViewingCollection(collection);
    setCurrentView('collection-view');
  };

  const handleCreateCollection = () => {
    setEditingCollection(undefined);
    setCurrentView('collection-form');
  };

  const handleEditCollection = () => {
    if (viewingCollection) {
      setEditingCollection(viewingCollection);
      setViewingCollection(undefined);
      setCurrentView('collection-form');
    }
  };

  const handleSaveCollection = (collection: Collection) => {
    setCurrentView('collections');
    setEditingCollection(undefined);
    setViewingCollection(undefined);
  };

  const handleCancelCollectionForm = () => {
    setCurrentView('collections');
    setEditingCollection(undefined);
  };

  const handleCloseCollectionView = () => {
    setCurrentView('collections');
    setViewingCollection(undefined);
  };

  const handleLogin = (user: User, token?: string) => {
    login(user, token);
    setShowAuth(null);
  };

  const handleRegister = (user: User, token?: string) => {
    login(user, token);
    setShowAuth(null);
  };

  const handleLogout = () => {
    logout();
    setCurrentView('list');
  };

  const handleShowLogin = () => setShowAuth('login');
  const handleShowRegister = () => setShowAuth('register');

  if (loading) {
    return (
      <div className="App">
        <div className="loading">
          <p>Loading...</p>
        </div>
      </div>
    );
  }

  if (!isAuthenticated) {
    return (
      <div className="App">
        <header className="App-header">
          <div className="header-content">
            <div className="header-title">
              <h1>RecipeForADisaster</h1>
              <p>AI-Powered Recipe Manager</p>
            </div>
            <nav className="header-nav">
              <div className="auth-buttons">
                <button onClick={handleShowLogin} className="auth-button">
                  Login
                </button>
                <button onClick={handleShowRegister} className="auth-button">
                  Register
                </button>
              </div>
            </nav>
          </div>
        </header>

        <main className="App-main">
          <div className="welcome-section">
            <h2>Welcome to RecipeForADisaster</h2>
            <p>Discover, create, and organize your favorite recipes with the power of AI.</p>
            <div className="welcome-actions">
              <button onClick={handleShowRegister} className="welcome-button primary">
                Get Started - Create Account
              </button>
              <button onClick={handleShowLogin} className="welcome-button secondary">
                Already have an account? Login
              </button>
            </div>
          </div>
        </main>

        {showAuth === 'login' && (
          <Login
            onLogin={handleLogin}
            onSwitchToRegister={handleShowRegister}
          />
        )}

        {showAuth === 'register' && (
          <Register
            onRegister={handleRegister}
            onSwitchToLogin={handleShowLogin}
          />
        )}

        <footer className="App-footer">
          <p>&copy; 2025 RecipeForADisaster - Powered by AI and Crow</p>
        </footer>
      </div>
    );
  }

  return (
    <div className="App">
      <header className="App-header">
        <div className="header-content">
          <div className="header-title">
            <h1>RecipeForADisaster</h1>
            <p>AI-Powered Recipe Manager</p>
          </div>
          <nav className="header-nav">
            {isAuthenticated ? (
              <>
                <button onClick={() => setCurrentView('list')} className="nav-button">
                  Recipes
                </button>
                <button onClick={handleViewCollections} className="nav-button">
                  Collections
                </button>
                <button onClick={handleViewProfile} className="nav-button">
                  Profile
                </button>
                <div className="user-info">
                  <span>Welcome, {user?.name || user?.email}</span>
                  <button onClick={handleLogout} className="logout-button">
                    Logout
                  </button>
                </div>
              </>
            ) : (
              <div className="auth-buttons">
                <button onClick={handleShowLogin} className="auth-button">
                  Login
                </button>
                <button onClick={handleShowRegister} className="auth-button">
                  Register
                </button>
              </div>
            )}
          </nav>
        </div>
      </header>

      <main className="App-main">
        {currentView === 'list' ? (
          <div className="list-view">
            <div className="actions-bar">
              <button onClick={handleAddRecipe} className="add-recipe-button">
                Add New Recipe
              </button>
              <button onClick={handleAIGeneration} className="ai-generate-button">
                Generate with AI
              </button>
            </div>
            <Suspense fallback={<div>Loading recipes...</div>}>
              <RecipeList
                onEdit={handleEditRecipe}
                onView={handleViewRecipe}
                refreshTrigger={refreshTrigger}
              />
            </Suspense>
          </div>
        ) : currentView === 'form' ? (
          <RecipeForm
            recipe={editingRecipe}
            onSave={handleSaveRecipe}
            onCancel={handleCancelForm}
          />
        ) : currentView === 'ai' ? (
          <AIGeneration onBack={() => setCurrentView('list')} />
        ) : currentView === 'profile' ? (
          <ProfileView
            onEdit={handleEditProfile}
            onClose={() => setCurrentView('list')}
          />
        ) : currentView === 'profile-edit' ? (
          <ProfileEdit
            onSave={handleSaveProfile}
            onCancel={handleCancelProfileEdit}
          />
        ) : currentView === 'collections' ? (
          <CollectionList
            onViewCollection={handleViewCollection}
            onCreateCollection={handleCreateCollection}
          />
        ) : currentView === 'collection-view' ? (
          viewingCollection && (
            <CollectionView
              collection={viewingCollection}
              onClose={handleCloseCollectionView}
              onEdit={handleEditCollection}
            />
          )
        ) : currentView === 'collection-form' ? (
          <CollectionForm
            collection={editingCollection}
            onSave={handleSaveCollection}
            onCancel={handleCancelCollectionForm}
          />
        ) : (
          viewingRecipe && (
            <Suspense fallback={<div>Loading recipe details...</div>}>
            <RecipeDetails
              recipe={viewingRecipe}
              onClose={handleCloseDetails}
              onEdit={handleEditFromDetails}
            />
            </Suspense>
          )
        )}
      </main>

      <footer className="App-footer">
        <p>&copy; 2025 RecipeForADisaster - Powered by AI and Crow</p>
      </footer>

      {/* PWA Components */}
      <PWAInstallPrompt />
      <OfflineIndicator />
    </div>
  );
}

function App() {
  return (
    <AuthProvider>
      <AppContent />
    </AuthProvider>
  );
}

export default App;

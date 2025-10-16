import React, { useState } from 'react';
import { recipeApi } from '../services/api';
import PromptBuilder from './PromptBuilder';
import RecipePreview from './RecipePreview';
import { PROMPT_EXAMPLES } from '../data/promptTemplates';
import { Recipe } from '../types/Recipe';

interface AIGenerationProps {
  onRecipeGenerated?: (recipe: any) => void;
  onBack?: () => void;
}

const AIGeneration: React.FC<AIGenerationProps> = ({ onRecipeGenerated, onBack }) => {
  const [prompt, setPrompt] = useState('');
  const [count, setCount] = useState(1);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [aiStatus, setAiStatus] = useState<any>(null);
  const [generationProgress, setGenerationProgress] = useState<string>('');
  const [generatedRecipe, setGeneratedRecipe] = useState<Partial<Recipe> | null>(null);
  const [showPreview, setShowPreview] = useState(false);

  const progressMessages = [
    'Analyzing your recipe request...',
    'Consulting culinary experts...',
    'Crafting the perfect recipe...',
    'Adding finishing touches...',
    'Recipe ready!'
  ];

  React.useEffect(() => {
    // Check AI service status on component mount
    checkAiStatus();
  }, []);

  const checkAiStatus = async () => {
    try {
      const status = await recipeApi.getAiStatus();
      setAiStatus(status);
    } catch (err) {
      console.error('Failed to check AI status:', err);
    }
  };

  const handleGenerate = async () => {
    if (!prompt.trim()) {
      setError('Please describe your recipe or select a template');
      return;
    }

    setLoading(true);
    setError(null);
    setGenerationProgress(progressMessages[0]);

    try {
      // Simulate progress updates
      const progressInterval = setInterval(() => {
        setGenerationProgress(prev => {
          const currentIndex = progressMessages.indexOf(prev);
          const nextIndex = Math.min(currentIndex + 1, progressMessages.length - 2);
          return progressMessages[nextIndex];
        });
      }, 1500);

      const result = await recipeApi.generateRecipe(prompt, count);

      clearInterval(progressInterval);
      setGenerationProgress(progressMessages[progressMessages.length - 1]);

      if (result.success) {
        // Small delay to show completion message
        setTimeout(() => {
          if (count === 1) {
            // Single recipe - parse and show preview
            const generatedRecipe = parseGeneratedRecipe(result.data.generatedRecipe);
            setGeneratedRecipe(generatedRecipe);
            setShowPreview(true);
          } else {
            // Multiple suggestions - show first one or let user choose
            if (result.data.suggestions && result.data.suggestions.length > 0) {
              const firstSuggestion = result.data.suggestions[0];
              if (firstSuggestion.success) {
                const generatedRecipe = parseGeneratedRecipe(firstSuggestion.content);
                setGeneratedRecipe(generatedRecipe);
                setShowPreview(true);
              } else {
                setError(firstSuggestion.error || 'Failed to generate recipe');
              }
            }
          }
        }, 1000);
      } else {
        setError(result.error || 'Failed to generate recipe');
      }
    } catch (err: any) {
      setError(err.response?.data?.message || 'Failed to generate recipe');
    } finally {
      setTimeout(() => {
        setLoading(false);
        setGenerationProgress('');
      }, 1500);
    }
  };

  const parseGeneratedRecipe = (generatedContent: string) => {
    // Simple parsing of AI-generated recipe content
    // This is a basic implementation - could be enhanced with better parsing
    const lines = generatedContent.split('\n');
    let title = '';
    let ingredients = '';
    let instructions = '';
    let category = 'Other';
    let type = 'Main Course';
    let servingSize = '4 servings';
    let cookTime = '30 minutes';

    let currentSection = '';

    for (const line of lines) {
      const trimmed = line.trim();
      if (!trimmed) continue;

      if (trimmed.toLowerCase().includes('title:') || trimmed.toLowerCase().includes('name:')) {
        title = trimmed.split(':')[1]?.trim() || trimmed;
        currentSection = 'title';
      } else if (trimmed.toLowerCase().includes('ingredients:')) {
        currentSection = 'ingredients';
      } else if (trimmed.toLowerCase().includes('instructions:') || trimmed.toLowerCase().includes('directions:')) {
        currentSection = 'instructions';
      } else if (trimmed.toLowerCase().includes('category:')) {
        category = trimmed.split(':')[1]?.trim() || category;
      } else if (trimmed.toLowerCase().includes('type:') || trimmed.toLowerCase().includes('course:')) {
        type = trimmed.split(':')[1]?.trim() || type;
      } else if (trimmed.toLowerCase().includes('servings:') || trimmed.toLowerCase().includes('serving size:')) {
        servingSize = trimmed.split(':')[1]?.trim() || servingSize;
      } else if (trimmed.toLowerCase().includes('cook time:') || trimmed.toLowerCase().includes('time:')) {
        cookTime = trimmed.split(':')[1]?.trim() || cookTime;
      } else {
        // Add content to current section
        if (currentSection === 'ingredients') {
          ingredients += (ingredients ? '\n' : '') + trimmed;
        } else if (currentSection === 'instructions') {
          instructions += (instructions ? '\n' : '') + trimmed;
        } else if (!title && trimmed.length > 0 && trimmed.length < 100) {
          // If no title yet and line looks like a title
          title = trimmed;
        }
      }
    }

    return {
      title: title || 'AI Generated Recipe',
      ingredients: ingredients || 'Ingredients not specified',
      instructions: instructions || 'Instructions not specified',
      category,
      type,
      servingSize,
      cookTime,
    };
  };

  const handleSaveRecipe = (recipe: Recipe) => {
    onRecipeGenerated?.(recipe);
    setShowPreview(false);
    setGeneratedRecipe(null);
    setPrompt(''); // Reset form
  };

  const handleEditRecipe = () => {
    // RecipePreview handles its own editing state
  };

  const handleCancelPreview = () => {
    setShowPreview(false);
    setGeneratedRecipe(null);
  };

  if (showPreview && generatedRecipe) {
    return (
      <div className="ai-generation">
        <RecipePreview
          recipe={generatedRecipe}
          onSave={handleSaveRecipe}
          onEdit={handleEditRecipe}
          onCancel={handleCancelPreview}
        />
      </div>
    );
  }

  return (
    <div className="ai-generation">
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '20px' }}>
        <h2>🤖 AI Recipe Generator</h2>
        {onBack && (
          <button onClick={onBack} className="back-button">
            ← Back to Recipes
          </button>
        )}
      </div>

      {aiStatus && (
        <div className={`ai-status ${aiStatus.aiServiceConfigured ? 'configured' : 'not-configured'}`}>
          <h3>AI Service Status</h3>
          <p>
            <strong>Configured:</strong> {aiStatus.aiServiceConfigured ? '✅ Yes' : '❌ No'}
          </p>
          {aiStatus.aiServiceConfigured && (
            <p>
              <strong>Connected:</strong> {aiStatus.aiServiceConnected ? '✅ Yes' : '❌ No'}
            </p>
          )}
          {!aiStatus.aiServiceConfigured && (
            <p className="status-help">
              {aiStatus.configurationHelp}
            </p>
          )}
        </div>
      )}

      <div className="ai-form">
        <div className="form-group">
          <label htmlFor="ai-prompt">Recipe Description:</label>
          <PromptBuilder
            onPromptChange={setPrompt}
            initialPrompt={prompt}
          />
        </div>

        <div className="form-group">
          <label>Or choose from examples:</label>
          <select
            value=""
            onChange={(e) => setPrompt(e.target.value)}
            disabled={!aiStatus?.aiServiceConfigured}
          >
            <option value="">Select an example...</option>
            {PROMPT_EXAMPLES.map((example, index) => (
              <option key={index} value={example}>{example}</option>
            ))}
          </select>
        </div>

        <div className="form-group">
          <label htmlFor="ai-count">Number of suggestions:</label>
          <select
            id="ai-count"
            value={count}
            onChange={(e) => setCount(Number(e.target.value))}
            disabled={!aiStatus?.aiServiceConfigured}
          >
            <option value={1}>1 Recipe</option>
            <option value={2}>2 Suggestions</option>
            <option value={3}>3 Suggestions</option>
            <option value={4}>4 Suggestions</option>
            <option value={5}>5 Suggestions</option>
          </select>
        </div>

        {error && <div className="error-message">{error}</div>}

        {loading && generationProgress && (
          <div className="loading-progress">
            <div className="loading-spinner"></div>
            <p>{generationProgress}</p>
          </div>
        )}

        <button
          onClick={handleGenerate}
          disabled={loading || !aiStatus?.aiServiceConfigured}
          className={`generate-button ${loading ? 'loading' : ''}`}
        >
          {loading ? (
            <>
              <div className="button-spinner"></div>
              Generating Recipe...
            </>
          ) : (
            `🤖 Generate ${count === 1 ? 'Recipe' : `${count} Suggestions`}`
          )}
        </button>

        {!aiStatus?.aiServiceConfigured && (
          <p className="ai-help">
            AI features require Azure OpenAI configuration. Set environment variables to enable.
          </p>
        )}
      </div>
    </div>
  );
};

export default AIGeneration;
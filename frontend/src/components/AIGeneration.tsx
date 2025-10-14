import React, { useState } from 'react';
import { recipeApi } from '../services/api';

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
      setError('Please enter a recipe description');
      return;
    }

    setLoading(true);
    setError(null);

    try {
      const result = await recipeApi.generateRecipe(prompt, count);

      if (result.success) {
        if (count === 1) {
          // Single recipe - parse and create recipe object
          const generatedRecipe = parseGeneratedRecipe(result.data.generatedRecipe);
          onRecipeGenerated?.(generatedRecipe);
        } else {
          // Multiple suggestions - show first one or let user choose
          if (result.data.suggestions && result.data.suggestions.length > 0) {
            const firstSuggestion = result.data.suggestions[0];
            if (firstSuggestion.success) {
              const generatedRecipe = parseGeneratedRecipe(firstSuggestion.content);
              onRecipeGenerated?.(generatedRecipe);
            } else {
              setError(firstSuggestion.error || 'Failed to generate recipe');
            }
          }
        }
      } else {
        setError(result.error || 'Failed to generate recipe');
      }
    } catch (err: any) {
      setError(err.response?.data?.message || 'Failed to generate recipe');
    } finally {
      setLoading(false);
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
          <label htmlFor="ai-prompt">Describe your recipe:</label>
          <textarea
            id="ai-prompt"
            value={prompt}
            onChange={(e) => setPrompt(e.target.value)}
            placeholder="e.g., A healthy vegetarian pasta dish with seasonal vegetables and herbs"
            rows={4}
            disabled={!aiStatus?.aiServiceConfigured}
          />
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

        <button
          onClick={handleGenerate}
          disabled={loading || !aiStatus?.aiServiceConfigured}
          className="generate-button"
        >
          {loading ? 'Generating...' : 'Generate Recipe with AI'}
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
import React, { useState } from 'react';
import { PROMPT_TEMPLATES, CUISINE_TYPES, DIETARY_RESTRICTIONS, DIFFICULTY_LEVELS, COOKING_TIME_OPTIONS } from '../data/promptTemplates';

interface PromptBuilderProps {
  onPromptChange: (prompt: string) => void;
  initialPrompt?: string;
}

const PromptBuilder: React.FC<PromptBuilderProps> = ({ onPromptChange, initialPrompt = '' }) => {
  const [selectedTemplate, setSelectedTemplate] = useState<string>('');
  const [cuisine, setCuisine] = useState<string>('');
  const [dietary, setDietary] = useState<string[]>([]);
  const [difficulty, setDifficulty] = useState<string>('');
  const [cookingTime, setCookingTime] = useState<string>('');
  const [customPrompt, setCustomPrompt] = useState<string>(initialPrompt);

  const handleTemplateSelect = (templateId: string) => {
    const template = PROMPT_TEMPLATES.find(t => t.id === templateId);
    if (template) {
      setSelectedTemplate(templateId);
      setCuisine(template.cuisine || '');
      setDietary(template.dietary || []);
      setDifficulty(template.difficulty || '');
      generatePrompt(template);
    }
  };

  const generatePrompt = (template?: any) => {
    const selectedTemplateData = template || PROMPT_TEMPLATES.find(t => t.id === selectedTemplate);

    if (!selectedTemplateData) {
      onPromptChange(customPrompt);
      return;
    }

    let prompt = selectedTemplateData.template;

    // Replace placeholders
    if (cuisine && cuisine !== 'any') {
      prompt = prompt.replace('{cuisine}', cuisine.toLowerCase());
    } else {
      prompt = prompt.replace('{cuisine}', 'any cuisine');
    }

    if (dietary.length > 0) {
      const dietaryText = dietary.join(' and ');
      prompt = prompt.replace('{dietary}', dietaryText.toLowerCase());
    } else {
      prompt = prompt.replace('{dietary}', 'regular');
    }

    if (difficulty) {
      prompt = prompt.replace('{difficulty}', difficulty);
    }

    // Add cooking time if specified
    if (cookingTime) {
      prompt += ` The total cooking time should be ${cookingTime.toLowerCase()}.`;
    }

    // Add custom prompt if provided
    if (customPrompt.trim()) {
      prompt += ` Additional requirements: ${customPrompt.trim()}`;
    }

    onPromptChange(prompt);
  };

  const handleDietaryToggle = (restriction: string) => {
    setDietary(prev =>
      prev.includes(restriction)
        ? prev.filter(r => r !== restriction)
        : [...prev, restriction]
    );
  };

  const handleCustomPromptChange = (value: string) => {
    setCustomPrompt(value);
    generatePrompt();
  };

  React.useEffect(() => {
    generatePrompt();
  }, [cuisine, dietary, difficulty, cookingTime, selectedTemplate]);

  return (
    <div className="prompt-builder">
      <div className="prompt-section">
        <h3>Choose a Recipe Style</h3>
        <div className="template-grid">
          {PROMPT_TEMPLATES.map(template => (
            <div
              key={template.id}
              className={`template-card ${selectedTemplate === template.id ? 'selected' : ''}`}
              onClick={() => handleTemplateSelect(template.id)}
            >
              <h4>{template.name}</h4>
              <p>{template.description}</p>
              <div className="template-tags">
                <span className="tag category">{template.category}</span>
                <span className="tag difficulty">{template.difficulty}</span>
              </div>
            </div>
          ))}
        </div>
      </div>

      <div className="prompt-section">
        <h3>Customize Your Recipe</h3>

        <div className="form-row">
          <div className="form-group">
            <label>Cuisine Type:</label>
            <select
              value={cuisine}
              onChange={(e) => setCuisine(e.target.value)}
            >
              <option value="">Any Cuisine</option>
              {CUISINE_TYPES.map(type => (
                <option key={type} value={type}>{type}</option>
              ))}
            </select>
          </div>

          <div className="form-group">
            <label>Difficulty Level:</label>
            <select
              value={difficulty}
              onChange={(e) => setDifficulty(e.target.value)}
            >
              <option value="">Any Difficulty</option>
              {DIFFICULTY_LEVELS.map(level => (
                <option key={level.value} value={level.value}>{level.label}</option>
              ))}
            </select>
          </div>
        </div>

        <div className="form-group">
          <label>Cooking Time:</label>
          <select
            value={cookingTime}
            onChange={(e) => setCookingTime(e.target.value)}
          >
            <option value="">Any Time</option>
            {COOKING_TIME_OPTIONS.map(time => (
              <option key={time} value={time}>{time}</option>
            ))}
          </select>
        </div>

        <div className="form-group">
          <label>Dietary Restrictions:</label>
          <div className="dietary-options">
            {DIETARY_RESTRICTIONS.map(restriction => (
              <label key={restriction} className="checkbox-label">
                <input
                  type="checkbox"
                  checked={dietary.includes(restriction)}
                  onChange={() => handleDietaryToggle(restriction)}
                />
                {restriction}
              </label>
            ))}
          </div>
        </div>
      </div>

      <div className="prompt-section">
        <h3>Additional Instructions (Optional)</h3>
        <textarea
          value={customPrompt}
          onChange={(e) => handleCustomPromptChange(e.target.value)}
          placeholder="Add any specific requirements, ingredients to include/exclude, or special instructions..."
          rows={3}
        />
      </div>
    </div>
  );
};

export default PromptBuilder;
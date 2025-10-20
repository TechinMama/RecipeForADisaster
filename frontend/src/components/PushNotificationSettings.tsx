import React, { useState, useEffect } from 'react';
import { usePushNotifications } from '../hooks/usePushNotifications';
import './PushNotificationSettings.css';

const PushNotificationSettings: React.FC = () => {
  const { isSupported, isSubscribed, subscribe, unsubscribe, sendTestNotification } = usePushNotifications();
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [success, setSuccess] = useState<string | null>(null);

  const handleToggleSubscription = async () => {
    setIsLoading(true);
    setError(null);
    setSuccess(null);

    try {
      if (isSubscribed) {
        await unsubscribe();
        setSuccess('Successfully unsubscribed from push notifications');
      } else {
        await subscribe();
        setSuccess('Successfully subscribed to push notifications');
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'An error occurred');
    } finally {
      setIsLoading(false);
    }
  };

  const handleTestNotification = async () => {
    setIsLoading(true);
    setError(null);

    try {
      await sendTestNotification();
      setSuccess('Test notification sent!');
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to send test notification');
    } finally {
      setIsLoading(false);
    }
  };

  // Clear messages after 5 seconds
  useEffect(() => {
    if (error || success) {
      const timer = setTimeout(() => {
        setError(null);
        setSuccess(null);
      }, 5000);
      return () => clearTimeout(timer);
    }
  }, [error, success]);

  if (!isSupported) {
    return (
      <div className="push-settings-container">
        <div className="push-settings-card">
          <h3>Push Notifications</h3>
          <p className="push-unsupported">
            Push notifications are not supported in your browser.
            Try using a modern browser like Chrome, Firefox, or Edge.
          </p>
        </div>
      </div>
    );
  }

  return (
    <div className="push-settings-container">
      <div className="push-settings-card">
        <h3>Push Notifications</h3>
        <p className="push-description">
          Get notified about new recipes, updates, and reminders from Recipe For A Disaster.
        </p>

        <div className="push-status">
          <div className={`status-indicator ${isSubscribed ? 'enabled' : 'disabled'}`}>
            {isSubscribed ? '🔔 Enabled' : '🔕 Disabled'}
          </div>
        </div>

        <div className="push-actions">
          <button
            onClick={handleToggleSubscription}
            disabled={isLoading}
            className={`push-toggle-btn ${isSubscribed ? 'unsubscribe' : 'subscribe'}`}
          >
            {isLoading ? 'Loading...' : (isSubscribed ? 'Unsubscribe' : 'Subscribe')}
          </button>

          {isSubscribed && (
            <button
              onClick={handleTestNotification}
              disabled={isLoading}
              className="push-test-btn"
            >
              Send Test Notification
            </button>
          )}
        </div>

        {error && (
          <div className="push-message error">
            <span>❌</span> {error}
          </div>
        )}

        {success && (
          <div className="push-message success">
            <span>✅</span> {success}
          </div>
        )}

        <div className="push-info">
          <h4>What you'll receive:</h4>
          <ul>
            <li>New recipe recommendations</li>
            <li>Updates to recipes you follow</li>
            <li>Weekly cooking reminders</li>
            <li>Community highlights</li>
          </ul>
        </div>
      </div>
    </div>
  );
};

export default PushNotificationSettings;
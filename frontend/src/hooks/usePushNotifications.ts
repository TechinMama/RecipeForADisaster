import { useState, useEffect } from 'react';

interface PushNotificationHook {
  isSupported: boolean;
  isSubscribed: boolean;
  subscribe: () => Promise<void>;
  unsubscribe: () => Promise<void>;
  sendTestNotification: () => Promise<void>;
}

export const usePushNotifications = (): PushNotificationHook => {
  const [isSupported, setIsSupported] = useState(false);
  const [isSubscribed, setIsSubscribed] = useState(false);
  const [registration, setRegistration] = useState<ServiceWorkerRegistration | null>(null);

  useEffect(() => {
    // Check if push notifications are supported
    if ('serviceWorker' in navigator && 'PushManager' in window) {
      setIsSupported(true);

      // Get the service worker registration
      navigator.serviceWorker.ready.then(reg => {
        setRegistration(reg);

        // Check if already subscribed
        reg.pushManager.getSubscription().then(subscription => {
          setIsSubscribed(!!subscription);
        });
      });
    }
  }, []);

  const subscribe = async (): Promise<void> => {
    if (!registration) return;

    try {
      // Request permission
      const permission = await Notification.requestPermission();
      if (permission !== 'granted') {
        throw new Error('Notification permission denied');
      }

      // You'll need to get this from your backend
      const vapidPublicKey = process.env.REACT_APP_VAPID_PUBLIC_KEY || '';

      // Subscribe to push notifications
      const subscription = await registration.pushManager.subscribe({
        userVisibleOnly: true,
        applicationServerKey: urlBase64ToUint8Array(vapidPublicKey) as any
      });

      // Send subscription to backend
      await fetch('/api/push/subscribe', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(subscription)
      });

      setIsSubscribed(true);
    } catch (error) {
      console.error('Failed to subscribe to push notifications:', error);
      throw error;
    }
  };

  const unsubscribe = async (): Promise<void> => {
    if (!registration) return;

    try {
      const subscription = await registration.pushManager.getSubscription();
      if (subscription) {
        await subscription.unsubscribe();

        // Remove subscription from backend
        await fetch('/api/push/unsubscribe', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify(subscription)
        });
      }

      setIsSubscribed(false);
    } catch (error) {
      console.error('Failed to unsubscribe from push notifications:', error);
      throw error;
    }
  };

  const sendTestNotification = async (): Promise<void> => {
    if (!registration) return;

    try {
      await registration.showNotification('Recipe For A Disaster', {
        body: 'This is a test notification! 🎉',
        icon: '/logo192.png',
        badge: '/logo192.png',
        data: {
          dateOfArrival: Date.now(),
          primaryKey: 1
        }
      });
    } catch (error) {
      console.error('Failed to send test notification:', error);
      throw error;
    }
  };

  return {
    isSupported,
    isSubscribed,
    subscribe,
    unsubscribe,
    sendTestNotification
  };
};

// Utility function to convert VAPID key
function urlBase64ToUint8Array(base64String: string): Uint8Array {
  const padding = '='.repeat((4 - base64String.length % 4) % 4);
  const base64 = (base64String + padding)
    .replace(/-/g, '+')
    .replace(/_/g, '/');

  const rawData = window.atob(base64);
  const outputArray = new Uint8Array(rawData.length);

  for (let i = 0; i < rawData.length; ++i) {
    outputArray[i] = rawData.charCodeAt(i);
  }
  return outputArray;
}
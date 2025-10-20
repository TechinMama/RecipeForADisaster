import React, { useState, useEffect } from 'react';
import './OfflineIndicator.css';

interface OfflineStatus {
  offline: boolean;
  pendingOperations: number;
}

const OfflineIndicator: React.FC = () => {
  const [isOnline, setIsOnline] = useState(navigator.onLine);
  const [pendingOperations, setPendingOperations] = useState(0);
  const [showIndicator, setShowIndicator] = useState(false);
  const [syncing, setSyncing] = useState(false);
  const [lastSyncResult, setLastSyncResult] = useState<{successful: number, failed: number, total: number} | null>(null);

  useEffect(() => {
    const handleOnline = () => {
      setIsOnline(true);
      setShowIndicator(true);
      checkOfflineStatus();
      triggerSyncIfNeeded();
    };

    const handleOffline = () => {
      setIsOnline(false);
      setShowIndicator(true);
      checkOfflineStatus();
    };

    const handleSyncCompleted = (event: MessageEvent) => {
      if (event.data?.type === 'SYNC_COMPLETED') {
        setLastSyncResult(event.data);
        setSyncing(false);
        setPendingOperations(prev => prev - event.data.successful);

        // Hide sync result after 5 seconds
        setTimeout(() => setLastSyncResult(null), 5000);
      }
    };

    window.addEventListener('online', handleOnline);
    window.addEventListener('offline', handleOffline);
    navigator.serviceWorker?.addEventListener('message', handleSyncCompleted);

    // Initial check
    checkOfflineStatus();

    // Show indicator initially if offline
    if (!navigator.onLine) {
      setShowIndicator(true);
    }

    return () => {
      window.removeEventListener('online', handleOnline);
      window.removeEventListener('offline', handleOffline);
      navigator.serviceWorker?.removeEventListener('message', handleSyncCompleted);
    };
  }, []);

  const checkOfflineStatus = async () => {
    if ('serviceWorker' in navigator && navigator.serviceWorker.controller) {
      try {
        const response = await sendMessageToSW({ type: 'GET_OFFLINE_STATUS' });
        if (response) {
          setPendingOperations(response.pendingOperations || 0);
        }
      } catch (error) {
        console.log('Could not check offline status:', error);
      }
    }
  };

  const triggerSyncIfNeeded = async () => {
    if (pendingOperations > 0 && 'serviceWorker' in navigator) {
      setSyncing(true);
      try {
        await sendMessageToSW({ type: 'TRIGGER_SYNC' });
      } catch (error) {
        console.log('Could not trigger sync:', error);
        setSyncing(false);
      }
    }
  };

  const sendMessageToSW = (message: any): Promise<any> => {
    return new Promise((resolve) => {
      if (!navigator.serviceWorker.controller) {
        resolve(null);
        return;
      }

      const messageChannel = new MessageChannel();
      messageChannel.port1.onmessage = (event) => {
        resolve(event.data);
      };

      navigator.serviceWorker.controller.postMessage(message, [messageChannel.port2]);
    });
  };

  const handleManualSync = async () => {
    if (!isOnline || syncing) return;

    setSyncing(true);
    try {
      await sendMessageToSW({ type: 'TRIGGER_SYNC' });
    } catch (error) {
      console.log('Manual sync failed:', error);
      setSyncing(false);
    }
  };

  if (!showIndicator && pendingOperations === 0 && !lastSyncResult) {
    return null;
  }

  return (
    <div className={`offline-indicator ${isOnline ? 'online-indicator' : ''} ${syncing ? 'syncing' : ''}`}>
      <div className="offline-content">
        <span className="status-text">
          {syncing ? 'Syncing recipes...' :
           !isOnline ? 'You are currently offline' :
           pendingOperations > 0 ? `${pendingOperations} recipe changes pending sync` :
           lastSyncResult ? `Synced ${lastSyncResult.successful} recipes` :
           'Back online - recipes synced!'}
        </span>

        {isOnline && pendingOperations > 0 && !syncing && (
          <button
            className="sync-button"
            onClick={handleManualSync}
            disabled={syncing}
          >
            Sync Now
          </button>
        )}

        {syncing && (
          <div className="sync-spinner"></div>
        )}
      </div>
    </div>
  );
};

export default OfflineIndicator;
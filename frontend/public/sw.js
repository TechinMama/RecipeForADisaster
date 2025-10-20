// Service Worker for RecipeForADisaster PWA
const CACHE_NAME = 'recipe-for-a-disaster-v1';
const STATIC_CACHE = 'recipe-static-v1';
const DYNAMIC_CACHE = 'recipe-dynamic-v1';
const API_CACHE = 'recipe-api-v1';
const OFFLINE_DB_NAME = 'RecipeOfflineDB';
const OFFLINE_DB_VERSION = 1;

// Resources to cache immediately
const STATIC_ASSETS = [
  '/',
  '/static/js/bundle.js',
  '/static/css/main.css',
  '/manifest.json',
  '/favicon.ico',
  '/logo192.png',
  '/logo512.png'
];

// API endpoints to cache
const API_ENDPOINTS = [
  '/api/health',
  '/api/recipes',
  '/api/collections'
];

// Install event - cache static assets
self.addEventListener('install', (event) => {
  console.log('[SW] Install event');
  event.waitUntil(
    Promise.all([
      caches.open(STATIC_CACHE)
        .then(cache => {
          console.log('[SW] Caching static assets');
          return cache.addAll(STATIC_ASSETS);
        }),
      initializeOfflineDB()
    ]).catch(error => {
      console.error('[SW] Error during install:', error);
    })
  );
  // Force activation of new service worker
  self.skipWaiting();
});

// Activate event - clean up old caches
self.addEventListener('activate', (event) => {
  console.log('[SW] Activate event');
  event.waitUntil(
    caches.keys().then(cacheNames => {
      return Promise.all(
        cacheNames.map(cacheName => {
          if (cacheName !== STATIC_CACHE && cacheName !== DYNAMIC_CACHE && cacheName !== API_CACHE) {
            console.log('[SW] Deleting old cache:', cacheName);
            return caches.delete(cacheName);
          }
        })
      );
    }).then(() => {
      // Take control of all clients immediately
      return self.clients.claim();
    })
  );
});

// Fetch event - handle requests
self.addEventListener('fetch', (event) => {
  const { request } = event;
  const url = new URL(request.url);

  // Handle API requests
  if (url.pathname.startsWith('/api/')) {
    event.respondWith(handleApiRequest(request));
    return;
  }

  // Handle static assets
  if (request.destination === 'script' || request.destination === 'style' ||
      request.destination === 'image' || request.destination === 'font') {
    event.respondWith(handleStaticRequest(request));
    return;
  }

  // Handle navigation requests
  if (request.mode === 'navigate') {
    event.respondWith(handleNavigationRequest(request));
    return;
  }

  // Default: try cache first, then network
  event.respondWith(
    caches.match(request)
      .then(response => {
        return response || fetch(request);
      })
  );
});

// Handle API requests with network-first strategy and offline queuing
async function handleApiRequest(request) {
  const url = new URL(request.url);

  try {
    // Try network first for API calls
    const response = await fetch(request);

    // Cache successful GET requests
    if (request.method === 'GET' && response.ok) {
      const cache = await caches.open(API_CACHE);
      cache.put(request, response.clone());

      // Also cache recipes in IndexedDB for offline access
      if (url.pathname === '/api/recipes' || url.pathname.startsWith('/api/recipes/')) {
        try {
          const data = await response.clone().json();
          await cacheRecipesInIndexedDB(data.data?.recipes || data);
        } catch (error) {
          console.log('[SW] Could not cache recipes in IndexedDB:', error);
        }
      }
    }

    return response;
  } catch (error) {
    console.log('[SW] Network request failed, going offline mode for:', request.url);

    // Network failed, try cache first
    const cachedResponse = await caches.match(request);
    if (cachedResponse) {
      return cachedResponse;
    }

    // For write operations when offline, queue them
    if (request.method !== 'GET') {
      await queueOfflineOperation(request);
      return new Response(JSON.stringify({
        success: true,
        message: 'Operation queued for when you come back online',
        offline: true,
        queued: true
      }), {
        status: 202, // Accepted
        headers: { 'Content-Type': 'application/json' }
      });
    }

    // Return offline response for read operations
    if (url.pathname === '/api/recipes' && request.method === 'GET') {
      try {
        const cachedRecipes = await getCachedRecipes();
        return new Response(JSON.stringify({
          data: { recipes: cachedRecipes },
          offline: true,
          cached: true
        }), {
          status: 200,
          headers: { 'Content-Type': 'application/json' }
        });
      } catch (error) {
        console.error('[SW] Error getting cached recipes:', error);
      }
    }

    return new Response(JSON.stringify({
      error: 'Offline',
      message: 'You are currently offline. Some features may not be available.',
      offline: true
    }), {
      status: 503,
      headers: { 'Content-Type': 'application/json' }
    });
  }
}

// Handle static assets with cache-first strategy
async function handleStaticRequest(request) {
  const cachedResponse = await caches.match(request);
  if (cachedResponse) {
    return cachedResponse;
  }

  try {
    const response = await fetch(request);
    if (response.ok) {
      const cache = await caches.open(STATIC_CACHE);
      cache.put(request, response.clone());
    }
    return response;
  } catch (error) {
    console.error('[SW] Error fetching static asset:', error);
    throw error;
  }
}

// Initialize IndexedDB for offline storage
async function initializeOfflineDB() {
  return new Promise((resolve, reject) => {
    const request = indexedDB.open(OFFLINE_DB_NAME, OFFLINE_DB_VERSION);

    request.onerror = () => {
      console.error('[SW] IndexedDB error:', request.error);
      reject(request.error);
    };

    request.onsuccess = () => {
      console.log('[SW] IndexedDB initialized');
      resolve(request.result);
    };

    request.onupgradeneeded = (event) => {
      const db = event.target.result;

      // Create object stores for offline operations
      if (!db.objectStoreNames.contains('pendingRecipes')) {
        const pendingRecipesStore = db.createObjectStore('pendingRecipes', { keyPath: 'id', autoIncrement: true });
        pendingRecipesStore.createIndex('timestamp', 'timestamp', { unique: false });
        pendingRecipesStore.createIndex('operation', 'operation', { unique: false });
      }

      if (!db.objectStoreNames.contains('cachedRecipes')) {
        const cachedRecipesStore = db.createObjectStore('cachedRecipes', { keyPath: 'id' });
        cachedRecipesStore.createIndex('lastModified', 'lastModified', { unique: false });
      }

      console.log('[SW] IndexedDB schema created');
    };
  });
}

// IndexedDB helper functions
async function openOfflineDB() {
  return new Promise((resolve, reject) => {
    const request = indexedDB.open(OFFLINE_DB_NAME, OFFLINE_DB_VERSION);
    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error);
  });
}

async function queueOfflineOperation(request) {
  try {
    const db = await openOfflineDB();
    const transaction = db.transaction(['pendingRecipes'], 'readwrite');
    const store = transaction.objectStore('pendingRecipes');

    // Clone request data for storage
    const operation = {
      url: request.url,
      method: request.method,
      headers: Object.fromEntries(request.headers.entries()),
      timestamp: Date.now(),
      operation: getOperationType(request)
    };

    // For POST/PUT/PATCH, try to get the body
    if (['POST', 'PUT', 'PATCH'].includes(request.method)) {
      try {
        const clonedRequest = request.clone();
        const body = await clonedRequest.text();
        operation.body = body;
      } catch (error) {
        console.log('[SW] Could not read request body for offline queuing');
      }
    }

    await new Promise((resolve, reject) => {
      const addRequest = store.add(operation);
      addRequest.onsuccess = () => resolve(addRequest.result);
      addRequest.onerror = () => reject(addRequest.error);
    });

    console.log('[SW] Operation queued for offline sync:', operation.operation);

    // Register background sync if supported
    if ('serviceWorker' in navigator && 'sync' in window.ServiceWorkerRegistration.prototype) {
      navigator.serviceWorker.ready.then(registration => {
        registration.sync.register('background-sync-recipes');
      });
    }

  } catch (error) {
    console.error('[SW] Error queuing offline operation:', error);
  }
}

function getOperationType(request) {
  const url = new URL(request.url);
  if (url.pathname.includes('/recipes')) {
    if (request.method === 'POST') return 'create-recipe';
    if (request.method === 'PUT') return 'update-recipe';
    if (request.method === 'DELETE') return 'delete-recipe';
  }
  return 'unknown-operation';
}

async function cacheRecipesInIndexedDB(recipes) {
  if (!Array.isArray(recipes)) return;

  try {
    const db = await openOfflineDB();
    const transaction = db.transaction(['cachedRecipes'], 'readwrite');
    const store = transaction.objectStore('cachedRecipes');

    for (const recipe of recipes) {
      recipe.lastModified = Date.now();
      await new Promise((resolve, reject) => {
        const putRequest = store.put(recipe);
        putRequest.onsuccess = () => resolve(putRequest.result);
        putRequest.onerror = () => reject(putRequest.error);
      });
    }

    console.log(`[SW] Cached ${recipes.length} recipes in IndexedDB`);
  } catch (error) {
    console.error('[SW] Error caching recipes in IndexedDB:', error);
  }
}

async function getCachedRecipes() {
  try {
    const db = await openOfflineDB();
    const transaction = db.transaction(['cachedRecipes'], 'readonly');
    const store = transaction.objectStore('cachedRecipes');

    return new Promise((resolve, reject) => {
      const getAllRequest = store.getAll();
      getAllRequest.onsuccess = () => resolve(getAllRequest.result || []);
      getAllRequest.onerror = () => reject(getAllRequest.error);
    });
  } catch (error) {
    console.error('[SW] Error getting cached recipes:', error);
    return [];
  }
}

// Handle messages from the main thread
self.addEventListener('message', (event) => {
  const { type, data } = event.data || {};

  if (type === 'SKIP_WAITING') {
    self.skipWaiting();
  } else if (type === 'GET_OFFLINE_STATUS') {
    // Check if we have pending operations
    getPendingOperationsCount().then(count => {
      event.ports[0].postMessage({
        offline: !navigator.onLine,
        pendingOperations: count
      });
    });
  } else if (type === 'TRIGGER_SYNC') {
    event.waitUntil(syncPendingRecipes());
  }
});

async function getPendingOperationsCount() {
  try {
    const db = await openOfflineDB();
    const transaction = db.transaction(['pendingRecipes'], 'readonly');
    const store = transaction.objectStore('pendingRecipes');

    return new Promise((resolve) => {
      const countRequest = store.count();
      countRequest.onsuccess = () => resolve(countRequest.result);
      countRequest.onerror = () => resolve(0);
    });
  } catch (error) {
    return 0;
  }
}

// Handle notification clicks
self.addEventListener('notificationclick', (event) => {
  console.log('[SW] Notification click:', event);
  event.notification.close();

  if (event.action === 'explore') {
    // Open the app
    event.waitUntil(
      clients.openWindow('/')
    );
  }
});

// Sync pending recipes when back online
async function syncPendingRecipes() {
  try {
    console.log('[SW] Starting background sync of pending recipes...');

    const db = await openOfflineDB();
    const transaction = db.transaction(['pendingRecipes'], 'readonly');
    const store = transaction.objectStore('pendingRecipes');
    const index = store.index('timestamp');

    const pendingOperations = await new Promise((resolve, reject) => {
      const getAllRequest = index.getAll();
      getAllRequest.onsuccess = () => resolve(getAllRequest.result || []);
      getAllRequest.onerror = () => reject(getAllRequest.error);
    });

    if (pendingOperations.length === 0) {
      console.log('[SW] No pending operations to sync');
      return;
    }

    console.log(`[SW] Found ${pendingOperations.length} pending operations to sync`);

    // Sort by timestamp to maintain order
    pendingOperations.sort((a, b) => a.timestamp - b.timestamp);

    const results = [];
    for (const operation of pendingOperations) {
      try {
        const result = await syncOperation(operation);
        results.push({ operation, success: true, result });

        // Remove from queue on success
        await removeOperationFromQueue(operation.id);
      } catch (error) {
        console.error('[SW] Failed to sync operation:', operation, error);
        results.push({ operation, success: false, error: error.message });

        // For some operations, we might want to retry later
        // For now, we'll remove failed operations to prevent infinite retries
        await removeOperationFromQueue(operation.id);
      }
    }

    // Notify clients about sync completion
    await notifyClientsOfSyncResults(results);

    console.log('[SW] Background sync completed');

  } catch (error) {
    console.error('[SW] Error during background sync:', error);
  }
}

async function syncOperation(operation) {
  const headers = new Headers(operation.headers);

  const requestInit = {
    method: operation.method,
    headers: headers
  };

  if (operation.body) {
    requestInit.body = operation.body;
  }

  const response = await fetch(operation.url, requestInit);

  if (!response.ok) {
    throw new Error(`HTTP ${response.status}: ${response.statusText}`);
  }

  return await response.json();
}

async function removeOperationFromQueue(operationId) {
  try {
    const db = await openOfflineDB();
    const transaction = db.transaction(['pendingRecipes'], 'readwrite');
    const store = transaction.objectStore('pendingRecipes');

    await new Promise((resolve, reject) => {
      const deleteRequest = store.delete(operationId);
      deleteRequest.onsuccess = () => resolve(deleteRequest.result);
      deleteRequest.onerror = () => reject(deleteRequest.error);
    });
  } catch (error) {
    console.error('[SW] Error removing operation from queue:', error);
  }
}

async function notifyClientsOfSyncResults(results) {
  const clients = await self.clients.matchAll();
  const successful = results.filter(r => r.success).length;
  const failed = results.filter(r => !r.success).length;

  for (const client of clients) {
    client.postMessage({
      type: 'SYNC_COMPLETED',
      successful,
      failed,
      total: results.length
    });
  }
}
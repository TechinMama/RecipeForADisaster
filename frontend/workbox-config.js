module.exports = {
  globDirectory: 'build/',
  globPatterns: [
    '**/*.{js,css,html,png,jpg,jpeg,gif,svg,ico,json}'
  ],
  swDest: 'build/sw.js',
  swSrc: 'public/sw.js',
  maximumFileSizeToCacheInBytes: 5 * 1024 * 1024, // 5MB
  runtimeCaching: [
    {
      urlPattern: /^https:\/\/api\./,
      handler: 'NetworkFirst',
      options: {
        cacheName: 'api-cache',
        expiration: {
          maxEntries: 100,
          maxAgeSeconds: 24 * 60 * 60 // 24 hours
        },
        cacheKeyWillBeUsed: async ({ request }) => {
          return `${request.url}?${Date.now()}`;
        }
      }
    },
    {
      urlPattern: /\.(?:png|jpg|jpeg|svg|gif|ico)$/,
      handler: 'CacheFirst',
      options: {
        cacheName: 'images-cache',
        expiration: {
          maxEntries: 200,
          maxAgeSeconds: 7 * 24 * 60 * 60 // 7 days
        }
      }
    },
    {
      urlPattern: /^https:\/\/fonts\./,
      handler: 'StaleWhileRevalidate',
      options: {
        cacheName: 'fonts-cache',
        expiration: {
          maxEntries: 10,
          maxAgeSeconds: 365 * 24 * 60 * 60 // 1 year
        }
      }
    }
  ],
  skipWaiting: true,
  clientsClaim: true,
  navigateFallback: '/index.html',
  navigateFallbackDenylist: [
    /^\/api\//
  ]
};
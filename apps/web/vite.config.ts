import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  define: {
    CESIUM_BASE_URL: JSON.stringify('/cesium'),
  },
  optimizeDeps: {
    include: ['cesium', 'mersenne-twister', 'urijs'],
  },
  build: {
    chunkSizeWarningLimit: 2_000,
  },
})

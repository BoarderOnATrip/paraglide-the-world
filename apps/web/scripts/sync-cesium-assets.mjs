import { cpSync, mkdirSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const scriptDirectory = dirname(fileURLToPath(import.meta.url))
const appRoot = dirname(scriptDirectory)
const cesiumBuildRoot = join(appRoot, 'node_modules', 'cesium', 'Build', 'Cesium')
const publicCesiumRoot = join(appRoot, 'public', 'cesium')
const runtimeDirectories = ['Assets', 'ThirdParty', 'Widgets', 'Workers']

mkdirSync(publicCesiumRoot, { recursive: true })

for (const directory of runtimeDirectories) {
  cpSync(join(cesiumBuildRoot, directory), join(publicCesiumRoot, directory), {
    recursive: true,
    force: true,
  })
}

console.log('Synced Cesium runtime assets into apps/web/public/cesium')

import { useEffect, useRef } from 'react'
import {
  Cartesian3,
  Cartographic,
  Cesium3DTileset,
  Color,
  ConstantPositionProperty,
  ConstantProperty,
  Credit,
  Entity,
  Google2DImageryProvider,
  HeadingPitchRange,
  HeadingPitchRoll,
  ImageryLayer,
  Math as CesiumMath,
  Matrix4,
  Resource,
  Transforms,
  Viewer,
} from 'cesium'
import 'cesium/Build/Cesium/Widgets/widgets.css'
import type { FlightSimState } from '../../sim/flight-model'
import type { FlightSite } from '../../sim/site-data'
import type { GoogleWorldMode } from '../../sim/world-mode'

type WorldStatus = 'config-needed' | 'loading' | 'ready' | 'error'

type GooglePhotorealisticWorldProps = {
  apiKey: string | null
  mode: GoogleWorldMode
  site: FlightSite
  flightState: FlightSimState
  onTerrainSample: (terrainHeightMeters: number | null) => void
  onWorldStatusChange: (status: WorldStatus, detail?: string) => void
}

const GOOGLE_3D_TILESET_ROOT_URL = 'https://tile.googleapis.com/v1/3dtiles/root.json'

type GliderRig = {
  canopyMain: Entity
  canopyTrim: Entity
  leftRiser: Entity
  rightRiser: Entity
  torso: Entity
  leftLeg: Entity
  rightLeg: Entity
  pilotHead: Entity
}

type ChaseCameraState = {
  headingDeg: number
  pitchDeg: number
  rangeMeters: number
}

const CANOPY_MAIN_LOCAL_POINTS = [
  new Cartesian3(-8.6, 1.4, 5.2),
  new Cartesian3(-6.2, 2.2, 5.9),
  new Cartesian3(-3.1, 2.9, 6.4),
  new Cartesian3(0, 3.1, 6.6),
  new Cartesian3(3.1, 2.9, 6.4),
  new Cartesian3(6.2, 2.2, 5.9),
  new Cartesian3(8.6, 1.4, 5.2),
]

const CANOPY_TRIM_LOCAL_POINTS = [
  new Cartesian3(-8.1, 1.6, 5.55),
  new Cartesian3(-5.6, 2.25, 6.05),
  new Cartesian3(-2.8, 2.75, 6.35),
  new Cartesian3(0, 2.95, 6.45),
  new Cartesian3(2.8, 2.75, 6.35),
  new Cartesian3(5.6, 2.25, 6.05),
  new Cartesian3(8.1, 1.6, 5.55),
]

const LEFT_RISER_LOCAL_POINTS = [
  new Cartesian3(-3.6, 2.65, 6.1),
  new Cartesian3(-0.45, 0.8, 1.1),
]

const RIGHT_RISER_LOCAL_POINTS = [
  new Cartesian3(3.6, 2.65, 6.1),
  new Cartesian3(0.45, 0.8, 1.1),
]

const TORSO_LOCAL_POINTS = [
  new Cartesian3(0, 0.85, 1.15),
  new Cartesian3(0, -0.25, -1.15),
]

const LEFT_LEG_LOCAL_POINTS = [
  new Cartesian3(0, -0.25, -1.15),
  new Cartesian3(-0.7, 0.55, -2.75),
]

const RIGHT_LEG_LOCAL_POINTS = [
  new Cartesian3(0, -0.25, -1.15),
  new Cartesian3(0.7, 0.55, -2.75),
]

const PILOT_HEAD_LOCAL_POINT = new Cartesian3(0, 1.05, 1.95)
const CAMERA_TARGET_LOCAL_POINT = new Cartesian3(0, 2.6, 1.9)
const DEFAULT_CAMERA_STATE: ChaseCameraState = {
  headingDeg: 0,
  pitchDeg: -13,
  rangeMeters: 98,
}

function clamp(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value))
}

function normalizeDegrees(value: number) {
  return ((value % 360) + 360) % 360
}

function smoothValue(current: number, target: number, factor: number) {
  return current + (target - current) * factor
}

function smoothDegrees(current: number, target: number, factor: number) {
  const delta = ((target - current + 540) % 360) - 180
  return normalizeDegrees(current + delta * factor)
}

function transformLocalPoint(modelMatrix: Matrix4, localPoint: Cartesian3) {
  return Matrix4.multiplyByPoint(modelMatrix, localPoint, new Cartesian3())
}

function transformLocalPoints(modelMatrix: Matrix4, localPoints: Cartesian3[]) {
  return localPoints.map((localPoint) => transformLocalPoint(modelMatrix, localPoint))
}

function getConfigNeededDetail(mode: GoogleWorldMode) {
  return mode === 'premium-3d'
    ? 'Set VITE_GOOGLE_MAPS_API_KEY to stream Google photorealistic 3D terrain.'
    : 'Set VITE_GOOGLE_MAPS_API_KEY to stream Google satellite tiles.'
}

function getWorldLoadingDetail(mode: GoogleWorldMode) {
  return mode === 'premium-3d'
    ? 'Streaming Google photorealistic 3D terrain...'
    : 'Streaming Google satellite tiles...'
}

function getWorldReadyDetail(mode: GoogleWorldMode, site: FlightSite) {
  return mode === 'premium-3d'
    ? `${site.name} photorealistic terrain loaded.`
    : `${site.name} satellite tiles loaded.`
}

export function GooglePhotorealisticWorld({
  apiKey,
  mode,
  site,
  flightState,
  onTerrainSample,
  onWorldStatusChange,
}: GooglePhotorealisticWorldProps) {
  const containerRef = useRef<HTMLDivElement | null>(null)
  const viewerRef = useRef<Viewer | null>(null)
  const tilesetRef = useRef<Cesium3DTileset | null>(null)
  const gliderRigRef = useRef<GliderRig | null>(null)
  const chaseCameraRef = useRef<ChaseCameraState>(DEFAULT_CAMERA_STATE)
  const isWorldReadyRef = useRef(false)

  useEffect(() => {
    if (!containerRef.current || viewerRef.current) {
      return
    }

    let viewer: Viewer

    try {
      viewer = new Viewer(containerRef.current, {
        animation: false,
        baseLayer: false,
        baseLayerPicker: false,
        fullscreenButton: false,
        geocoder: false,
        homeButton: false,
        infoBox: false,
        navigationHelpButton: false,
        sceneModePicker: false,
        selectionIndicator: false,
        timeline: false,
        skyBox: false,
        shouldAnimate: true,
        scene3DOnly: true,
        shadows: mode === 'premium-3d',
      })
    } catch (error) {
      const detail =
        error instanceof Error
          ? error.message
          : 'Cesium could not initialize WebGL for this device or browser.'

      isWorldReadyRef.current = false
      onWorldStatusChange('error', detail)
      return
    }

    viewer.scene.globe.show = mode === 'standard-2d'
    viewer.scene.globe.enableLighting = mode === 'premium-3d'
    viewer.scene.fog.enabled = true
    if (viewer.scene.skyAtmosphere) {
      viewer.scene.skyAtmosphere.show = true
      viewer.scene.skyAtmosphere.brightnessShift = -0.08
    }
    viewer.scene.backgroundColor = Color.fromCssColorString('#86b5dd')
    viewer.scene.postProcessStages.fxaa.enabled = true
    viewer.scene.screenSpaceCameraController.enableCollisionDetection = false
    const creditContainer = viewer.cesiumWidget.creditContainer as HTMLElement
    creditContainer.style.pointerEvents = 'none'
    viewer.creditDisplay.addStaticCredit(new Credit('Google Maps Platform data'))

    const initialGliderPosition = Cartesian3.fromDegrees(
      site.longitude,
      site.latitude,
      site.launchAltitudeMeters + site.spawnAglMeters,
    )
    const pilotHeadColor = Color.fromCssColorString('#f8fafc')
    const harnessColor = Color.fromCssColorString('#111827')
    const canopyColor = Color.fromCssColorString('#fff7ed')
    const canopyTrimColor = Color.fromCssColorString('#fb923c')

    gliderRigRef.current = {
      canopyMain: viewer.entities.add({
        polyline: {
          positions: [initialGliderPosition, initialGliderPosition],
          width: 11,
          material: canopyColor,
        },
      }),
      canopyTrim: viewer.entities.add({
        polyline: {
          positions: [initialGliderPosition, initialGliderPosition],
          width: 4.5,
          material: canopyTrimColor,
        },
      }),
      leftRiser: viewer.entities.add({
        polyline: {
          positions: [initialGliderPosition, initialGliderPosition],
          width: 1.7,
          material: pilotHeadColor.withAlpha(0.9),
        },
      }),
      rightRiser: viewer.entities.add({
        polyline: {
          positions: [initialGliderPosition, initialGliderPosition],
          width: 1.7,
          material: pilotHeadColor.withAlpha(0.9),
        },
      }),
      torso: viewer.entities.add({
        polyline: {
          positions: [initialGliderPosition, initialGliderPosition],
          width: 4.2,
          material: harnessColor,
        },
      }),
      leftLeg: viewer.entities.add({
        polyline: {
          positions: [initialGliderPosition, initialGliderPosition],
          width: 3.2,
          material: harnessColor,
        },
      }),
      rightLeg: viewer.entities.add({
        polyline: {
          positions: [initialGliderPosition, initialGliderPosition],
          width: 3.2,
          material: harnessColor,
        },
      }),
      pilotHead: viewer.entities.add({
        position: initialGliderPosition,
        point: {
          pixelSize: 8,
          color: pilotHeadColor,
          outlineColor: harnessColor,
          outlineWidth: 2,
          disableDepthTestDistance: Number.POSITIVE_INFINITY,
        },
      }),
    }

    viewerRef.current = viewer
    chaseCameraRef.current = {
      ...DEFAULT_CAMERA_STATE,
      headingDeg: site.prevailingWindHeadingDeg,
    }

    return () => {
      viewer.destroy()
      viewerRef.current = null
      tilesetRef.current = null
      gliderRigRef.current = null
      chaseCameraRef.current = DEFAULT_CAMERA_STATE
    }
  }, [
    mode,
    onWorldStatusChange,
    site.latitude,
    site.launchAltitudeMeters,
    site.longitude,
    site.prevailingWindHeadingDeg,
    site.spawnAglMeters,
  ])

  useEffect(() => {
    const viewer = viewerRef.current

    if (!viewer) {
      return
    }

    if (!apiKey) {
      isWorldReadyRef.current = false
      onWorldStatusChange('config-needed', getConfigNeededDetail(mode))
      return
    }

    let cancelled = false

    async function loadWorld() {
      onWorldStatusChange('loading', getWorldLoadingDetail(mode))

      async function loadSatelliteImagery(detail: string) {
        const imageryProvider = await Google2DImageryProvider.fromUrl({
          key: apiKey ?? undefined,
          mapType: 'satellite',
        })
        const activeViewer = viewerRef.current

        if (cancelled || !activeViewer) {
          return false
        }

        activeViewer.scene.globe.show = true
        activeViewer.scene.globe.enableLighting = false
        activeViewer.imageryLayers.removeAll(true)
        activeViewer.imageryLayers.add(
          new ImageryLayer(
            imageryProvider as unknown as ConstructorParameters<typeof ImageryLayer>[0],
          ),
        )
        isWorldReadyRef.current = true
        onTerrainSample(site.launchAltitudeMeters)
        onWorldStatusChange('ready', detail)
        return true
      }

      try {
        if (mode === 'standard-2d') {
          await loadSatelliteImagery(getWorldReadyDetail(mode, site))
          return
        }

        const resource = new Resource({
          url: GOOGLE_3D_TILESET_ROOT_URL,
          queryParameters: {
            key: apiKey,
          },
        })

        const tileset = await Cesium3DTileset.fromUrl(resource, {
          dynamicScreenSpaceError: true,
          maximumScreenSpaceError: 18,
          skipLevelOfDetail: true,
        })
        const activeViewer = viewerRef.current

        if (cancelled || !activeViewer) {
          return
        }

        activeViewer.scene.globe.show = false
        activeViewer.scene.globe.enableLighting = true
        activeViewer.scene.primitives.add(tileset)
        tilesetRef.current = tileset
        isWorldReadyRef.current = true
        onWorldStatusChange('ready', getWorldReadyDetail(mode, site))
      } catch (error) {
        if (cancelled) {
          return
        }

        const detail =
          error instanceof Error
            ? error.message
            : mode === 'premium-3d'
              ? 'Unable to load Google photorealistic 3D Tiles.'
              : 'Unable to load Google satellite tiles.'

        if (mode === 'premium-3d') {
          try {
            const fallbackLoaded = await loadSatelliteImagery(
              `Premium 3D failed, showing Google satellite tiles instead. ${detail}`,
            )

            if (fallbackLoaded) {
              return
            }
          } catch {
            // Keep the original 3D error if the fallback also fails.
          }
        }

        isWorldReadyRef.current = false
        onWorldStatusChange('error', detail)
      }
    }

    void loadWorld()

    return () => {
      cancelled = true
    }
  }, [apiKey, mode, onTerrainSample, onWorldStatusChange, site])

  useEffect(() => {
    const viewer = viewerRef.current
    const gliderRig = gliderRigRef.current

    if (!viewer || !gliderRig) {
      return
    }

    const gliderPosition = Cartesian3.fromDegrees(
      flightState.longitude,
      flightState.latitude,
      flightState.altitudeMeters,
    )
    const gliderFrame = Transforms.headingPitchRollToFixedFrame(
      gliderPosition,
      new HeadingPitchRoll(
        CesiumMath.toRadians(flightState.headingDeg),
        CesiumMath.toRadians(flightState.pitchDeg * 0.6),
        CesiumMath.toRadians(-flightState.bankDeg),
      ),
    )
    const canopyMainPositions = transformLocalPoints(
      gliderFrame,
      CANOPY_MAIN_LOCAL_POINTS,
    )
    const canopyTrimPositions = transformLocalPoints(
      gliderFrame,
      CANOPY_TRIM_LOCAL_POINTS,
    )

    gliderRig.canopyMain.polyline!.positions = new ConstantProperty(canopyMainPositions)
    gliderRig.canopyTrim.polyline!.positions = new ConstantProperty(canopyTrimPositions)
    gliderRig.leftRiser.polyline!.positions = new ConstantProperty(transformLocalPoints(
      gliderFrame,
      LEFT_RISER_LOCAL_POINTS,
    ))
    gliderRig.rightRiser.polyline!.positions = new ConstantProperty(transformLocalPoints(
      gliderFrame,
      RIGHT_RISER_LOCAL_POINTS,
    ))
    gliderRig.torso.polyline!.positions = new ConstantProperty(transformLocalPoints(
      gliderFrame,
      TORSO_LOCAL_POINTS,
    ))
    gliderRig.leftLeg.polyline!.positions = new ConstantProperty(transformLocalPoints(
      gliderFrame,
      LEFT_LEG_LOCAL_POINTS,
    ))
    gliderRig.rightLeg.polyline!.positions = new ConstantProperty(transformLocalPoints(
      gliderFrame,
      RIGHT_LEG_LOCAL_POINTS,
    ))
    gliderRig.pilotHead.position = new ConstantPositionProperty(
      transformLocalPoint(gliderFrame, PILOT_HEAD_LOCAL_POINT),
    )

    const chaseCamera = chaseCameraRef.current
    const targetHeadingDeg = normalizeDegrees(flightState.headingDeg)
    const targetPitchDeg = clamp(
      -11 - Math.abs(flightState.bankDeg) * 0.08 - Math.max(0, -flightState.pitchDeg) * 0.22,
      -18,
      -10,
    )
    const targetRangeMeters = clamp(
      88 +
        Math.max(0, flightState.groundSpeedKmh - 28) * 0.85 +
        Math.abs(flightState.bankDeg) * 0.45,
      92,
      122,
    )
    const nextCameraState: ChaseCameraState = {
      headingDeg: smoothDegrees(
        chaseCamera.headingDeg,
        targetHeadingDeg,
        0.24,
      ),
      pitchDeg: smoothValue(chaseCamera.pitchDeg, targetPitchDeg, 0.22),
      rangeMeters: smoothValue(
        chaseCamera.rangeMeters,
        targetRangeMeters,
        0.18,
      ),
    }

    chaseCameraRef.current = nextCameraState

    viewer.camera.lookAt(
      transformLocalPoint(gliderFrame, CAMERA_TARGET_LOCAL_POINT),
      new HeadingPitchRange(
        CesiumMath.toRadians(nextCameraState.headingDeg),
        CesiumMath.toRadians(nextCameraState.pitchDeg),
        nextCameraState.rangeMeters,
      ),
    )

    if (
      isWorldReadyRef.current &&
      mode === 'premium-3d' &&
      viewer.scene.sampleHeightSupported
    ) {
      const terrainHeight = viewer.scene.sampleHeight(
        Cartographic.fromDegrees(
          flightState.longitude,
          flightState.latitude,
          flightState.altitudeMeters,
        ),
      )

      onTerrainSample(terrainHeight ?? null)
    }

    viewer.scene.requestRender()
  }, [
    flightState.altitudeMeters,
    flightState.bankDeg,
    flightState.groundSpeedKmh,
    flightState.headingDeg,
    flightState.latitude,
    flightState.longitude,
    flightState.pitchDeg,
    mode,
    onTerrainSample,
  ])

  return <div ref={containerRef} className="sim-world" />
}

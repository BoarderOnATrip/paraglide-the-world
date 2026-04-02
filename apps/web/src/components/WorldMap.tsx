import { useEffect, useRef, useState } from 'react'
import maplibregl, { type GeoJSONSource, type Map as MapLibreMap } from 'maplibre-gl'
import 'maplibre-gl/dist/maplibre-gl.css'
import type { ActivityModeId } from '../data'
import type { MissionCheckpointView } from '../lib/mission-engine'
import type { CountryContent } from '../lib/typing-content'

type WorldMapProps = {
  activityModeId: ActivityModeId
  country: CountryContent
  checkpoints: MissionCheckpointView[]
  progress: number
  isActiveFlight: boolean
  isPaused: boolean
  windHeading: number
  windSpeedKmh: number
  speedKmh: number
}

const STYLE_URL = 'https://demotiles.maplibre.org/globe.json'
const ROUTE_SOURCE_ID = 'paraglide-route'
const CHECKPOINT_SOURCE_ID = 'paraglide-checkpoints'
const MARKER_SOURCE_ID = 'paraglide-marker'

function createLoopCoordinates(
  center: [number, number],
  radiusKm: number,
  segments = 64,
) {
  const [lng, lat] = center
  const latRadius = radiusKm / 110.574
  const lngRadius = radiusKm / (111.32 * Math.cos((lat * Math.PI) / 180))

  return Array.from({ length: segments + 1 }, (_, index) => {
    const angle = (index / segments) * Math.PI * 2

    return [
      lng + lngRadius * Math.cos(angle),
      lat + latRadius * Math.sin(angle),
    ] as [number, number]
  })
}

function getPointOnLoop(center: [number, number], radiusKm: number, progress: number) {
  const [lng, lat] = center
  const latRadius = radiusKm / 110.574
  const lngRadius = radiusKm / (111.32 * Math.cos((lat * Math.PI) / 180))
  const angle = progress * Math.PI * 2

  return [
    lng + lngRadius * Math.cos(angle),
    lat + latRadius * Math.sin(angle),
  ] as [number, number]
}

function getCameraForMode(country: CountryContent, activityModeId: ActivityModeId) {
  const { camera } = country

  switch (activityModeId) {
    case 'paragliding':
      return camera
    case 'scenic-flight':
      return {
        center: camera.center,
        zoom: Math.max(6.8, camera.zoom - 1.1),
        pitch: 48,
        bearing: camera.bearing - 8,
      }
    case 'boat-tour':
      return {
        center: camera.center,
        zoom: camera.zoom + 0.65,
        pitch: 54,
        bearing: camera.bearing + 16,
      }
    case 'car-ride':
      return {
        center: camera.center,
        zoom: camera.zoom + 1.35,
        pitch: 66,
        bearing: camera.bearing + 24,
      }
    default:
      return camera
  }
}

function getRouteRadiusKm(activityModeId: ActivityModeId) {
  return activityModeId === 'paragliding'
    ? 24
    : activityModeId === 'scenic-flight'
      ? 18
      : activityModeId === 'boat-tour'
        ? 12
        : 8
}

function updateMapSources(
  map: MapLibreMap,
  country: CountryContent,
  checkpoints: MissionCheckpointView[],
  activityModeId: ActivityModeId,
  progress: number,
) {
  const radiusKm = getRouteRadiusKm(activityModeId)
  const routeCoordinates = createLoopCoordinates(country.camera.center, radiusKm)
  const currentPoint = getPointOnLoop(
    country.camera.center,
    radiusKm,
    Math.max(progress, 0.001),
  )

  const routeSource = map.getSource(ROUTE_SOURCE_ID) as GeoJSONSource | undefined
  routeSource?.setData({
    type: 'Feature',
    geometry: {
      type: 'LineString',
      coordinates: routeCoordinates,
    },
    properties: {
      mode: activityModeId,
    },
  })

  const checkpointSource = map.getSource(CHECKPOINT_SOURCE_ID) as
    | GeoJSONSource
    | undefined
  checkpointSource?.setData({
    type: 'FeatureCollection',
    features: checkpoints.map((checkpoint) => ({
      type: 'Feature',
      geometry: {
        type: 'Point',
        coordinates: getPointOnLoop(
          country.camera.center,
          radiusKm,
          Math.min(Math.max(checkpoint.progress, 0.001), 0.999),
        ),
      },
      properties: {
        label: checkpoint.label,
        status: checkpoint.status,
      },
    })),
  })

  const markerSource = map.getSource(MARKER_SOURCE_ID) as GeoJSONSource | undefined
  markerSource?.setData({
    type: 'Feature',
    geometry: {
      type: 'Point',
      coordinates: currentPoint,
    },
    properties: {
      country: country.name,
      mode: activityModeId,
    },
  })
}

export function WorldMap({
  activityModeId,
  country,
  checkpoints,
  progress,
  isActiveFlight,
  isPaused,
  windHeading,
  windSpeedKmh,
  speedKmh,
}: WorldMapProps) {
  const containerRef = useRef<HTMLDivElement | null>(null)
  const mapRef = useRef<MapLibreMap | null>(null)
  const [mapStatus, setMapStatus] = useState<'loading' | 'ready' | 'error'>('loading')

  useEffect(() => {
    if (!containerRef.current || mapRef.current) {
      return
    }

    const map = new maplibregl.Map({
      container: containerRef.current,
      style: STYLE_URL,
      center: country.camera.center,
      zoom: 1.8,
      pitch: 20,
      bearing: 0,
    })

    mapRef.current = map

    map.addControl(new maplibregl.NavigationControl({ visualizePitch: true }), 'top-right')

    map.on('load', () => {
      map.setProjection({ type: 'globe' })

      map.addSource(ROUTE_SOURCE_ID, {
        type: 'geojson',
        data: {
          type: 'Feature',
          geometry: { type: 'LineString', coordinates: [] },
          properties: {},
        },
      })

      map.addLayer({
        id: ROUTE_SOURCE_ID,
        type: 'line',
        source: ROUTE_SOURCE_ID,
        paint: {
          'line-color': '#ffe8a3',
          'line-width': 3,
          'line-opacity': 0.8,
        },
      })

      map.addSource(CHECKPOINT_SOURCE_ID, {
        type: 'geojson',
        data: {
          type: 'FeatureCollection',
          features: [],
        },
      })

      map.addLayer({
        id: CHECKPOINT_SOURCE_ID,
        type: 'circle',
        source: CHECKPOINT_SOURCE_ID,
        paint: {
          'circle-radius': [
            'match',
            ['get', 'status'],
            'active',
            8,
            'cleared',
            6,
            5,
          ],
          'circle-color': [
            'match',
            ['get', 'status'],
            'active',
            '#ffd369',
            'cleared',
            '#fff3ca',
            '#244e6d',
          ],
          'circle-opacity': 0.96,
          'circle-stroke-width': 2,
          'circle-stroke-color': '#fffbef',
        },
      })

      map.addSource(MARKER_SOURCE_ID, {
        type: 'geojson',
        data: {
          type: 'Feature',
          geometry: { type: 'Point', coordinates: country.camera.center },
          properties: {},
        },
      })

      map.addLayer({
        id: MARKER_SOURCE_ID,
        type: 'circle',
        source: MARKER_SOURCE_ID,
        paint: {
          'circle-radius': 8,
          'circle-color': '#f8f0dc',
          'circle-stroke-width': 3,
          'circle-stroke-color': '#244e6d',
        },
      })

      updateMapSources(map, country, checkpoints, activityModeId, progress)
      map.easeTo({
        ...getCameraForMode(country, activityModeId),
        duration: 2200,
      })
      setMapStatus('ready')
    })

    map.on('error', () => {
      setMapStatus('error')
    })

    return () => {
      map.remove()
      mapRef.current = null
    }
  }, [activityModeId, checkpoints, country, progress])

  useEffect(() => {
    const map = mapRef.current

    if (!map || !map.isStyleLoaded()) {
      return
    }

    updateMapSources(map, country, checkpoints, activityModeId, progress)
  }, [activityModeId, checkpoints, country, progress])

  useEffect(() => {
    const map = mapRef.current

    if (!map || !map.isStyleLoaded()) {
      return
    }

    map.easeTo({
      ...getCameraForMode(country, activityModeId),
      duration: 1600,
      essential: true,
    })
  }, [activityModeId, country])

  return (
    <div className="world-map">
      <div ref={containerRef} className="world-map__canvas" />
      <div className="world-map__hud">
        <span>
          {isPaused ? 'Flight Paused' : isActiveFlight ? 'Active Lift' : 'Scenic Traverse'}
        </span>
        <span>{country.name}</span>
        <span>
          Next {checkpoints.find((checkpoint) => checkpoint.status === 'active')?.label ?? 'Loop'}
        </span>
        <span>{Math.round(progress * 100)}% route energy</span>
        <span>{speedKmh} km/h groundspeed</span>
        <span>
          {windSpeedKmh} km/h wind · {windHeading}°
        </span>
      </div>
      {mapStatus === 'ready' && isPaused ? (
        <div className="world-map__state">
          Route hold active. Open the hangout deck to text, then resume when you
          want the world moving again.
        </div>
      ) : null}
      {mapStatus === 'loading' ? (
        <div className="world-map__state">Loading free world shell...</div>
      ) : null}
      {mapStatus === 'error' ? (
        <div className="world-map__state world-map__state--error">
          Globe rendering failed. The prototype will still run without the world
          layer.
        </div>
      ) : null}
    </div>
  )
}

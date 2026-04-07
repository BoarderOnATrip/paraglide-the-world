import { createInitialFlightState } from './physics'
import {
  getAngleDeltaDegrees,
  offsetPositionByMeters,
  toRadians,
  wrapDegrees,
} from './geometry'
import type { AmbientAirState, FlightSimState } from './types'
import type { FlightSite, FlightSiteId, RidgeBand } from '../sim/site-data'
import { getFlightSite } from '../sim/site-data'

export type FlightScenarioId =
  | 'ridge-pass'
  | 'thermal-climb'
  | 'glide-transition'
  | 'approach'
  | 'flare'

export type FlightScenarioPreset = {
  id: FlightScenarioId
  name: string
  siteId: FlightSiteId
  summary: string
  setup: string
  recommendedInputs: string
  keyOutputs: string[]
}

export type FlightScenarioSession = {
  scenario: FlightScenarioPreset
  site: FlightSite
  terrainHeightMeters: number
  atmosphere: AmbientAirState
  flightState: FlightSimState
}

export const FLIGHT_SCENARIOS: FlightScenarioPreset[] = [
  {
    id: 'ridge-pass',
    name: 'Ridge Pass',
    siteId: 'lauterbrunnen',
    summary: 'Start in the windward band and read progressive ridge lift versus lee sink.',
    setup: 'Medium ridge wind, stable entry speed, and enough clearance to work the face.',
    recommendedInputs: 'Fly a clean pass first, then explore brake and weight shift near the ridge band.',
    keyOutputs: ['ridge lift', 'net vario', 'turn authority'],
  },
  {
    id: 'thermal-climb',
    name: 'Thermal Climb',
    siteId: 'lauterbrunnen',
    summary: 'Enter a seeded thermal edge and feel how centering changes climb quality.',
    setup: 'Light-to-medium wind, drifting core, and enough height to circle without rushing.',
    recommendedInputs: 'Use gentle coordinated turns and watch the vario improve as you center.',
    keyOutputs: ['thermal lift', 'sink ring', 'climb stability'],
  },
  {
    id: 'glide-transition',
    name: 'Glide Transition',
    siteId: 'rome',
    summary: 'Neutral air prototype for comparing trim, brake, and speed-bar glide tradeoffs.',
    setup: 'High enough to experiment, low turbulence, and only a little wind penetration pressure.',
    recommendedInputs: 'Fly trim, then compare deep brake and speed bar without changing the line too much.',
    keyOutputs: ['airspeed range', 'glide trend', 'stall warning'],
  },
  {
    id: 'approach',
    name: 'Approach',
    siteId: 'istanbul',
    summary: 'Short final setup to read alignment, energy, and landing-zone guidance before touchdown.',
    setup: 'Low altitude, mild headwind, and a landing line that rewards stable alignment.',
    recommendedInputs: 'Stay coordinated, read the landing error numbers, and avoid big late inputs.',
    keyOutputs: ['groundspeed', 'vertical speed', 'approach error'],
  },
  {
    id: 'flare',
    name: 'Flare',
    siteId: 'istanbul',
    summary: 'Final meters prototype for comparing early, timed, and late flare timing.',
    setup: 'Very low final, committed landing line, and enough speed for timing to matter.',
    recommendedInputs: 'Use symmetric brake timing near the ground and watch flare effectiveness versus landing rating.',
    keyOutputs: ['flare effectiveness', 'touchdown speed', 'landing rating'],
  },
] as const

function getWindwardNormalHeadingDeg(ridge: RidgeBand, windHeadingDeg: number) {
  const normalA = wrapDegrees(ridge.axisHeadingDeg + 90)
  const normalB = wrapDegrees(ridge.axisHeadingDeg - 90)
  const upwindHeadingDeg = wrapDegrees(windHeadingDeg + 180)

  return Math.abs(getAngleDeltaDegrees(normalA, upwindHeadingDeg)) <=
    Math.abs(getAngleDeltaDegrees(normalB, upwindHeadingDeg))
    ? normalA
    : normalB
}

function offsetByHeading(
  latitude: number,
  longitude: number,
  headingDeg: number,
  distanceMeters: number,
) {
  const radians = toRadians(headingDeg)

  return offsetPositionByMeters(
    latitude,
    longitude,
    Math.sin(radians) * distanceMeters,
    Math.cos(radians) * distanceMeters,
  )
}

function getScenarioAtmosphere(
  scenarioId: FlightScenarioId,
  site: FlightSite,
): AmbientAirState {
  switch (scenarioId) {
    case 'ridge-pass':
      return {
        windHeadingDeg: site.prevailingWindHeadingDeg,
        windSpeedKmh: Math.max(site.windSpeedKmh, 24),
        turbulence: 0.1,
      }
    case 'thermal-climb':
      return {
        windHeadingDeg: site.prevailingWindHeadingDeg,
        windSpeedKmh: Math.max(site.windSpeedKmh - 3, 12),
        turbulence: 0.08,
      }
    case 'glide-transition':
      return {
        windHeadingDeg: site.prevailingWindHeadingDeg,
        windSpeedKmh: 6,
        turbulence: 0.04,
      }
    case 'approach':
      return {
        windHeadingDeg: site.prevailingWindHeadingDeg,
        windSpeedKmh: 14,
        turbulence: 0.06,
      }
    case 'flare':
      return {
        windHeadingDeg: site.prevailingWindHeadingDeg,
        windSpeedKmh: 10,
        turbulence: 0.04,
      }
  }
}

function withAgl(
  state: FlightSimState,
  terrainHeightMeters: number,
  aglMeters: number,
): FlightSimState {
  return {
    ...state,
    terrainHeightMeters,
    altitudeMeters: terrainHeightMeters + aglMeters,
    groundClearanceMeters: aglMeters,
  }
}

function createScenarioFlightState(
  scenarioId: FlightScenarioId,
  site: FlightSite,
  terrainHeightMeters: number,
  atmosphere: AmbientAirState,
): FlightSimState {
  const baseState = createInitialFlightState(site, terrainHeightMeters)

  switch (scenarioId) {
    case 'ridge-pass': {
      if (!site.ridge) {
        return withAgl(baseState, terrainHeightMeters, site.spawnAglMeters)
      }

      const windwardNormalHeadingDeg = getWindwardNormalHeadingDeg(
        site.ridge,
        atmosphere.windHeadingDeg,
      )
      const alongPosition = offsetByHeading(
        site.ridge.latitude,
        site.ridge.longitude,
        site.ridge.axisHeadingDeg,
        -site.ridge.lengthMeters * 0.14,
      )
      const position = offsetByHeading(
        alongPosition.latitude,
        alongPosition.longitude,
        windwardNormalHeadingDeg,
        site.ridge.windwardDepthMeters * 0.42,
      )

      return {
        ...withAgl(baseState, terrainHeightMeters, 135),
        latitude: position.latitude,
        longitude: position.longitude,
        headingDeg: site.ridge.axisHeadingDeg,
        airspeedKmh: 39,
        groundSpeedKmh: 39,
        verticalSpeedMetersPerSecond: 0.4,
        flightPhase: 'soaring',
      }
    }
    case 'thermal-climb': {
      const thermal = site.thermals[0]

      if (!thermal) {
        return withAgl(baseState, terrainHeightMeters, site.spawnAglMeters + 30)
      }

      const position = offsetByHeading(
        thermal.latitude,
        thermal.longitude,
        wrapDegrees(site.prevailingWindHeadingDeg + 90),
        thermal.radiusMeters * 0.58,
      )

      return {
        ...withAgl(baseState, terrainHeightMeters, 180),
        latitude: position.latitude,
        longitude: position.longitude,
        headingDeg: wrapDegrees(site.prevailingWindHeadingDeg + 28),
        airspeedKmh: 37,
        groundSpeedKmh: 34,
        verticalSpeedMetersPerSecond: 0.2,
        flightPhase: 'soaring',
      }
    }
    case 'glide-transition':
      return {
        ...withAgl(baseState, terrainHeightMeters, 180),
        headingDeg: site.prevailingWindHeadingDeg,
        airspeedKmh: 38,
        groundSpeedKmh: 36,
        verticalSpeedMetersPerSecond: -0.8,
        flightPhase: 'soaring',
      }
    case 'approach': {
      const landingZone = site.landingZone

      if (!landingZone) {
        return {
          ...withAgl(baseState, terrainHeightMeters, 18),
          airspeedKmh: 31,
          groundSpeedKmh: 26,
          verticalSpeedMetersPerSecond: -1.1,
          flightPhase: 'approach',
        }
      }

      const behindApproach = offsetByHeading(
        landingZone.latitude,
        landingZone.longitude,
        wrapDegrees(landingZone.headingDeg + 180),
        240,
      )
      const position = offsetByHeading(
        behindApproach.latitude,
        behindApproach.longitude,
        wrapDegrees(landingZone.headingDeg + 90),
        18,
      )

      return {
        ...withAgl(baseState, terrainHeightMeters, 18),
        latitude: position.latitude,
        longitude: position.longitude,
        headingDeg: wrapDegrees(landingZone.headingDeg - 8),
        airspeedKmh: 31,
        groundSpeedKmh: 25,
        verticalSpeedMetersPerSecond: -1.2,
        flightPhase: 'approach',
      }
    }
    case 'flare': {
      const landingZone = site.landingZone

      if (!landingZone) {
        return {
          ...withAgl(baseState, terrainHeightMeters, 2.8),
          airspeedKmh: 27,
          groundSpeedKmh: 24,
          verticalSpeedMetersPerSecond: -1.4,
          flightPhase: 'approach',
        }
      }

      const position = offsetByHeading(
        landingZone.latitude,
        landingZone.longitude,
        wrapDegrees(landingZone.headingDeg + 180),
        34,
      )

      return {
        ...withAgl(baseState, terrainHeightMeters, 2.8),
        latitude: position.latitude,
        longitude: position.longitude,
        headingDeg: landingZone.headingDeg,
        airspeedKmh: 27,
        groundSpeedKmh: 24,
        verticalSpeedMetersPerSecond: -1.3,
        flightPhase: 'approach',
      }
    }
  }
}

export function getFlightScenario(scenarioId: FlightScenarioId | null) {
  if (scenarioId == null) {
    return null
  }

  return FLIGHT_SCENARIOS.find((scenario) => scenario.id === scenarioId) ?? null
}

export function createFlightScenarioSession(
  scenarioId: FlightScenarioId,
  terrainHeightMeters?: number | null,
): FlightScenarioSession {
  const scenario = getFlightScenario(scenarioId)

  if (!scenario) {
    throw new Error(`Unknown flight scenario: ${scenarioId}`)
  }

  const site = getFlightSite(scenario.siteId)
  const resolvedTerrainHeightMeters =
    terrainHeightMeters ?? site.launchAltitudeMeters
  const atmosphere = getScenarioAtmosphere(scenario.id, site)
  const flightState = createScenarioFlightState(
    scenario.id,
    site,
    resolvedTerrainHeightMeters,
    atmosphere,
  )

  return {
    scenario,
    site,
    terrainHeightMeters: resolvedTerrainHeightMeters,
    atmosphere,
    flightState,
  }
}

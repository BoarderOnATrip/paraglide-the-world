import type { HomeRowControlState } from './home-row-controls'
import { derivesFromHomeRowControls } from './home-row-controls'
import type { FlightAssistProfile } from './flight-assist'
import type { FlightSite, LandingZone, RidgeBand, ThermalSource } from './site-data'

const METERS_PER_LATITUDE_DEGREE = 111_320
const TRIM_AIRSPEED_KMH = 38
const MIN_AIRSPEED_KMH = 18
const MAX_AIRSPEED_KMH = 55

export type FlightPhase =
  | 'launch'
  | 'soaring'
  | 'approach'
  | 'flare'
  | 'landed'
  | 'crashed'

export type LandingRating = 'none' | 'smooth' | 'firm' | 'hard' | 'crash'

export type FlightSimState = {
  latitude: number
  longitude: number
  altitudeMeters: number
  terrainHeightMeters: number
  headingDeg: number
  bankDeg: number
  pitchDeg: number
  airspeedKmh: number
  groundSpeedKmh: number
  verticalSpeedMetersPerSecond: number
  ridgeLiftMetersPerSecond: number
  thermalLiftMetersPerSecond: number
  airMassSinkMetersPerSecond: number
  groundClearanceMeters: number
  distanceKm: number
  elapsedSeconds: number
  turnRateDegPerSecond: number
  stallWarning: number
  flareEffectiveness: number
  landingZoneDistanceMeters: number | null
  landingApproachErrorDeg: number | null
  landingRating: LandingRating
  flightPhase: FlightPhase
}

export type AmbientAirState = {
  windHeadingDeg: number
  windSpeedKmh: number
  turbulence: number
}

type FlightStepInput = {
  deltaSeconds: number
  controls: HomeRowControlState
  site: FlightSite
  terrainHeightMeters: number | null
  atmosphere?: AmbientAirState | null
  assist?: FlightAssistProfile | null
}

type LocalOffsetMeters = {
  east: number
  north: number
}

type LandingZoneMetrics = {
  distanceMeters: number
  approachErrorDeg: number
  isInside: boolean
}

function clamp(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value))
}

function wrapDegrees(value: number) {
  return (value % 360 + 360) % 360
}

function approach(current: number, target: number, maxStep: number) {
  if (current < target) {
    return Math.min(current + maxStep, target)
  }

  return Math.max(current - maxStep, target)
}

function toRadians(value: number) {
  return (value * Math.PI) / 180
}

function toDegrees(value: number) {
  return (value * 180) / Math.PI
}

function getAngleDeltaDegrees(a: number, b: number) {
  return ((a - b + 540) % 360) - 180
}

function getMetersPerLongitudeDegree(latitude: number) {
  return Math.max(
    1,
    METERS_PER_LATITUDE_DEGREE * Math.cos((latitude * Math.PI) / 180),
  )
}

function getLocalOffsetMeters(
  latitude: number,
  longitude: number,
  centerLatitude: number,
  centerLongitude: number,
): LocalOffsetMeters {
  return {
    north: (latitude - centerLatitude) * METERS_PER_LATITUDE_DEGREE,
    east:
      (longitude - centerLongitude) *
      getMetersPerLongitudeDegree((latitude + centerLatitude) / 2),
  }
}

function getGroundDistanceMeters(
  latitudeA: number,
  longitudeA: number,
  latitudeB: number,
  longitudeB: number,
) {
  const offset = getLocalOffsetMeters(latitudeB, longitudeB, latitudeA, longitudeA)
  return Math.hypot(offset.north, offset.east)
}

function projectOffsetMeters(offset: LocalOffsetMeters, headingDeg: number) {
  const headingRadians = toRadians(headingDeg)
  return (
    offset.east * Math.sin(headingRadians) +
    offset.north * Math.cos(headingRadians)
  )
}

function offsetPositionByMeters(
  latitude: number,
  longitude: number,
  eastMeters: number,
  northMeters: number,
) {
  return {
    latitude: latitude + northMeters / METERS_PER_LATITUDE_DEGREE,
    longitude:
      longitude + eastMeters / getMetersPerLongitudeDegree(latitude),
  }
}

function getWindwardNormalHeadingDeg(ridge: RidgeBand, windHeadingDeg: number) {
  const normalA = wrapDegrees(ridge.axisHeadingDeg + 90)
  const normalB = wrapDegrees(ridge.axisHeadingDeg - 90)
  const upwindHeadingDeg = wrapDegrees(windHeadingDeg + 180)

  return Math.abs(getAngleDeltaDegrees(normalA, upwindHeadingDeg)) <=
    Math.abs(getAngleDeltaDegrees(normalB, upwindHeadingDeg))
    ? normalA
    : normalB
}

function getRidgeAirMass(
  latitude: number,
  longitude: number,
  windHeadingDeg: number,
  windSpeedKmh: number,
  site: FlightSite,
) {
  if (!site.ridge) {
    const distanceFromLaunchMeters = getGroundDistanceMeters(
      latitude,
      longitude,
      site.latitude,
      site.longitude,
    )
    const ridgeAlignment =
      Math.cos(toRadians(windHeadingDeg - site.prevailingWindHeadingDeg))
    const terrainWindow = clamp(1 - distanceFromLaunchMeters / 4_500, 0, 1)
    const windFactor = clamp((windSpeedKmh - 8) / 16, 0, 1.35)

    return {
      ridgeLiftMetersPerSecond:
        Math.max(0, ridgeAlignment) *
        site.baseRidgeLiftMetersPerSecond *
        terrainWindow *
        windFactor,
      leeSinkMetersPerSecond: 0,
    }
  }

  const windFactor = clamp((windSpeedKmh - 8) / 14, 0, 1.45)
  const offset = getLocalOffsetMeters(
    latitude,
    longitude,
    site.ridge.latitude,
    site.ridge.longitude,
  )
  const alongMeters = projectOffsetMeters(offset, site.ridge.axisHeadingDeg)
  const windwardNormalHeadingDeg = getWindwardNormalHeadingDeg(
    site.ridge,
    windHeadingDeg,
  )
  const crossWindwardMeters = projectOffsetMeters(offset, windwardNormalHeadingDeg)
  const alongFactor = clamp(
    1 - Math.abs(alongMeters) / (site.ridge.lengthMeters / 2),
    0,
    1,
  )

  if (alongFactor <= 0) {
    return {
      ridgeLiftMetersPerSecond: 0,
      leeSinkMetersPerSecond: 0,
    }
  }

  if (crossWindwardMeters >= 0) {
    const depthFactor = clamp(
      1 - crossWindwardMeters / site.ridge.windwardDepthMeters,
      0,
      1,
    )

    return {
      ridgeLiftMetersPerSecond:
        site.ridge.peakLiftMetersPerSecond *
        alongFactor *
        depthFactor *
        windFactor,
      leeSinkMetersPerSecond: 0,
    }
  }

  const leeFactor = clamp(
    1 - Math.abs(crossWindwardMeters) / site.ridge.leeDepthMeters,
    0,
    1,
  )

  return {
    ridgeLiftMetersPerSecond: 0,
    leeSinkMetersPerSecond:
      site.ridge.leeSinkMetersPerSecond * alongFactor * leeFactor * windFactor,
  }
}

function getDriftedThermalCenter(
  thermal: ThermalSource,
  atmosphere: AmbientAirState,
  elapsedSeconds: number,
) {
  const driftScale = thermal.driftFactor ?? 0.28
  const driftMeters =
    (atmosphere.windSpeedKmh / 3.6) * elapsedSeconds * driftScale
  const headingRadians = toRadians(atmosphere.windHeadingDeg)

  return offsetPositionByMeters(
    thermal.latitude,
    thermal.longitude,
    Math.sin(headingRadians) * driftMeters,
    Math.cos(headingRadians) * driftMeters,
  )
}

function getThermalAirMass(
  latitude: number,
  longitude: number,
  elapsedSeconds: number,
  atmosphere: AmbientAirState,
  site: FlightSite,
) {
  return site.thermals.reduce(
    (accumulator, thermal) => {
      const thermalCenter = getDriftedThermalCenter(
        thermal,
        atmosphere,
        elapsedSeconds,
      )
      const distanceMeters = getGroundDistanceMeters(
        latitude,
        longitude,
        thermalCenter.latitude,
        thermalCenter.longitude,
      )
      const coreRadiusMeters = thermal.coreRadiusMeters ?? thermal.radiusMeters * 0.35

      if (distanceMeters <= coreRadiusMeters) {
        const coreFactor = clamp(1 - distanceMeters / coreRadiusMeters, 0, 1)
        return {
          liftMetersPerSecond:
            accumulator.liftMetersPerSecond +
            thermal.liftMetersPerSecond * (0.7 + coreFactor * 0.3),
          sinkMetersPerSecond: accumulator.sinkMetersPerSecond,
        }
      }

      if (distanceMeters <= thermal.radiusMeters) {
        const edgeFactor = clamp(
          1 - (distanceMeters - coreRadiusMeters) /
            Math.max(thermal.radiusMeters - coreRadiusMeters, 1),
          0,
          1,
        )

        return {
          liftMetersPerSecond:
            accumulator.liftMetersPerSecond +
            thermal.liftMetersPerSecond * edgeFactor * edgeFactor * 0.72,
          sinkMetersPerSecond: accumulator.sinkMetersPerSecond,
        }
      }

      const sinkRingMetersPerSecond = thermal.sinkRingMetersPerSecond ?? 0
      const sinkRingRadiusMeters = thermal.radiusMeters * 1.45

      if (sinkRingMetersPerSecond <= 0 || distanceMeters > sinkRingRadiusMeters) {
        return accumulator
      }

      const sinkFactor = clamp(
        1 - (distanceMeters - thermal.radiusMeters) /
          Math.max(sinkRingRadiusMeters - thermal.radiusMeters, 1),
        0,
        1,
      )

      return {
        liftMetersPerSecond: accumulator.liftMetersPerSecond,
        sinkMetersPerSecond:
          accumulator.sinkMetersPerSecond +
          sinkRingMetersPerSecond * sinkFactor,
      }
    },
    {
      liftMetersPerSecond: 0,
      sinkMetersPerSecond: 0,
    },
  )
}

function getTurbulenceVerticalGust(
  elapsedSeconds: number,
  latitude: number,
  longitude: number,
  turbulence: number,
) {
  const phase = elapsedSeconds * 1.55 + latitude * 34 + longitude * 41
  const secondaryPhase = elapsedSeconds * 2.7 + latitude * 12 - longitude * 19

  return (
    (Math.sin(phase) * 0.62 + Math.sin(secondaryPhase) * 0.38) *
    turbulence *
    0.78
  )
}

function getTargetAirspeedKmh(
  symmetricBrake: number,
  speedBarTravel: number,
  assist: FlightAssistProfile,
) {
  const targetAirspeed =
    TRIM_AIRSPEED_KMH +
    speedBarTravel * 10.5 -
    symmetricBrake * 12.5 -
    symmetricBrake * symmetricBrake * 5.5 +
    (assist.inputResponsiveness - 0.9) * 4

  return clamp(targetAirspeed, MIN_AIRSPEED_KMH, MAX_AIRSPEED_KMH)
}

function getTurnRateDegPerSecond(bankDeg: number, airspeedKmh: number) {
  const airspeedMetersPerSecond = Math.max(airspeedKmh / 3.6, 6)
  const bankRadians = toRadians(bankDeg)

  return toDegrees((9.81 * Math.tan(bankRadians)) / airspeedMetersPerSecond)
}

function getLandingZoneMetrics(
  latitude: number,
  longitude: number,
  headingDeg: number,
  landingZone: LandingZone | undefined,
): LandingZoneMetrics | null {
  if (!landingZone) {
    return null
  }

  const offset = getLocalOffsetMeters(
    latitude,
    longitude,
    landingZone.latitude,
    landingZone.longitude,
  )
  const alongMeters = projectOffsetMeters(offset, landingZone.headingDeg)
  const crossMeters = projectOffsetMeters(offset, landingZone.headingDeg + 90)
  const outsideAlongMeters = Math.max(0, Math.abs(alongMeters) - landingZone.lengthMeters / 2)
  const outsideCrossMeters = Math.max(0, Math.abs(crossMeters) - landingZone.widthMeters / 2)

  return {
    distanceMeters: Math.hypot(outsideAlongMeters, outsideCrossMeters),
    approachErrorDeg: Math.abs(getAngleDeltaDegrees(headingDeg, landingZone.headingDeg)),
    isInside:
      Math.abs(alongMeters) <= landingZone.lengthMeters / 2 &&
      Math.abs(crossMeters) <= landingZone.widthMeters / 2,
  }
}

function getLandingRating(
  verticalSpeedMetersPerSecond: number,
  groundSpeedKmh: number,
  bankDeg: number,
  landingMetrics: LandingZoneMetrics | null,
) {
  const severityScore =
    Math.abs(verticalSpeedMetersPerSecond) * 2.35 +
    Math.max(0, groundSpeedKmh - 12) / 5 +
    Math.abs(bankDeg) / 10 +
    (landingMetrics ? landingMetrics.approachErrorDeg / 32 : 0) +
    (landingMetrics ? landingMetrics.distanceMeters / 38 : 0)

  if (severityScore < 2.4) {
    return 'smooth'
  }

  if (severityScore < 3.8) {
    return 'firm'
  }

  if (severityScore < 5.4) {
    return 'hard'
  }

  return 'crash'
}

export function createInitialFlightState(
  site: FlightSite,
  terrainHeightMeters: number = site.launchAltitudeMeters,
): FlightSimState {
  return {
    latitude: site.latitude,
    longitude: site.longitude,
    altitudeMeters: terrainHeightMeters + site.spawnAglMeters,
    terrainHeightMeters,
    headingDeg: site.prevailingWindHeadingDeg,
    bankDeg: 0,
    pitchDeg: -6,
    airspeedKmh: TRIM_AIRSPEED_KMH,
    groundSpeedKmh: TRIM_AIRSPEED_KMH,
    verticalSpeedMetersPerSecond: 0.1,
    ridgeLiftMetersPerSecond: site.baseRidgeLiftMetersPerSecond,
    thermalLiftMetersPerSecond: 0,
    airMassSinkMetersPerSecond: 0,
    groundClearanceMeters: site.spawnAglMeters,
    distanceKm: 0,
    elapsedSeconds: 0,
    turnRateDegPerSecond: 0,
    stallWarning: 0,
    flareEffectiveness: 0,
    landingZoneDistanceMeters: null,
    landingApproachErrorDeg: null,
    landingRating: 'none',
    flightPhase: 'launch',
  }
}

export function resetFlightStateForSite(
  site: FlightSite,
  terrainHeightMeters: number | null,
) {
  return createInitialFlightState(
    site,
    terrainHeightMeters ?? site.launchAltitudeMeters,
  )
}

export function stepFlightState(
  currentState: FlightSimState,
  input: FlightStepInput,
): FlightSimState {
  const controls = derivesFromHomeRowControls(input.controls)
  const assist = input.assist ?? {
    inputResponsiveness: 0.9,
    coordinationAssist: 0,
    turbulenceDamping: 0,
    recoveryAssist: 0,
  }
  const atmosphere = input.atmosphere ?? {
    windHeadingDeg: input.site.prevailingWindHeadingDeg,
    windSpeedKmh: input.site.windSpeedKmh,
    turbulence: 0.16,
  }
  const terrainHeightMeters =
    input.terrainHeightMeters ?? currentState.terrainHeightMeters

  if (currentState.flightPhase === 'landed' || currentState.flightPhase === 'crashed') {
    return {
      ...currentState,
      altitudeMeters: terrainHeightMeters,
      terrainHeightMeters,
      airspeedKmh: 0,
      groundSpeedKmh: 0,
      verticalSpeedMetersPerSecond: 0,
      groundClearanceMeters: 0,
      bankDeg: approach(currentState.bankDeg, 0, input.deltaSeconds * 18),
      pitchDeg: approach(currentState.pitchDeg, 0, input.deltaSeconds * 12),
      turnRateDegPerSecond: 0,
      elapsedSeconds: currentState.elapsedSeconds + input.deltaSeconds,
    }
  }

  const effectiveTurbulence = clamp(
    atmosphere.turbulence * (1 - assist.turbulenceDamping),
    0,
    1,
  )
  const targetAirspeedKmh = getTargetAirspeedKmh(
    controls.symmetricBrake,
    controls.speedBarTravel,
    assist,
  )
  let airspeedKmh = approach(
    currentState.airspeedKmh,
    targetAirspeedKmh,
    input.deltaSeconds * 16 * assist.inputResponsiveness,
  )
  const stallWarning = clamp(
    (controls.symmetricBrake - 0.72) / 0.24 +
      (23 - airspeedKmh) / 8 -
      controls.speedBarTravel * 0.18 -
      assist.recoveryAssist * 0.65,
    0,
    1,
  )
  const controlAuthority = clamp(
    assist.inputResponsiveness +
      assist.coordinationAssist * 0.42 -
      stallWarning * 0.38,
    0.62,
    1.16,
  )
  const gustBankDeg =
    Math.sin(currentState.elapsedSeconds * 1.9 + currentState.headingDeg / 23) *
    effectiveTurbulence *
    4.4
  const targetBankDeg = clamp(
    (controls.brakeDifferential * 28 + controls.weightShift * 12) * controlAuthority +
      gustBankDeg,
    -46,
    46,
  )
  const bankDeg = approach(
    currentState.bankDeg,
    targetBankDeg,
    input.deltaSeconds * (30 + assist.coordinationAssist * 34),
  )
  const turnRateDegPerSecond =
    getTurnRateDegPerSecond(bankDeg, airspeedKmh) * (1 - stallWarning * 0.24)
  const headingDeg = wrapDegrees(
    currentState.headingDeg + turnRateDegPerSecond * input.deltaSeconds,
  )
  const ridgeAirMass = getRidgeAirMass(
    currentState.latitude,
    currentState.longitude,
    atmosphere.windHeadingDeg,
    atmosphere.windSpeedKmh,
    input.site,
  )
  const thermalAirMass = getThermalAirMass(
    currentState.latitude,
    currentState.longitude,
    currentState.elapsedSeconds,
    atmosphere,
    input.site,
  )
  const turbulenceLiftMetersPerSecond = getTurbulenceVerticalGust(
    currentState.elapsedSeconds,
    currentState.latitude,
    currentState.longitude,
    effectiveTurbulence,
  )
  const speedDeviation = airspeedKmh - TRIM_AIRSPEED_KMH
  const baseSinkMetersPerSecond = 1.02 + (speedDeviation * speedDeviation) / 150
  const bankRadians = toRadians(bankDeg)
  const loadFactor = 1 / Math.max(Math.cos(bankRadians), 0.74)
  const inducedTurnSinkMetersPerSecond = Math.max(0, loadFactor - 1) * 0.95
  const brakeSinkMetersPerSecond =
    controls.symmetricBrake * 0.42 +
    controls.symmetricBrake * controls.symmetricBrake * 0.65
  const stallSinkMetersPerSecond = stallWarning * stallWarning * 3.3
  const lowAltitudeFactor = clamp(
    (8 - currentState.groundClearanceMeters) / 8,
    0,
    1,
  )
  const flareCommand = clamp((controls.symmetricBrake - 0.58) / 0.32, 0, 1)
  const flareEffectiveness =
    flareCommand * lowAltitudeFactor * (1 - stallWarning * 0.3)
  const flareLiftMetersPerSecond =
    flareEffectiveness * clamp((5 - currentState.groundClearanceMeters) / 5, 0, 1) * 3.4
  const flareDragKmh =
    flareEffectiveness * clamp((7 - currentState.groundClearanceMeters) / 7, 0, 1) * 10

  airspeedKmh = clamp(airspeedKmh - flareDragKmh, 9, MAX_AIRSPEED_KMH)

  const airMassSinkMetersPerSecond =
    ridgeAirMass.leeSinkMetersPerSecond + thermalAirMass.sinkMetersPerSecond
  const verticalSpeedMetersPerSecond =
    thermalAirMass.liftMetersPerSecond +
    ridgeAirMass.ridgeLiftMetersPerSecond +
    turbulenceLiftMetersPerSecond +
    flareLiftMetersPerSecond -
    (baseSinkMetersPerSecond +
      inducedTurnSinkMetersPerSecond +
      brakeSinkMetersPerSecond +
      stallSinkMetersPerSecond +
      airMassSinkMetersPerSecond)
  const targetPitchDeg = clamp(
    -6 +
      controls.speedBarTravel * 3.8 -
      controls.symmetricBrake * 8.4 +
      stallWarning * 8.8 +
      flareEffectiveness * 6.2 -
      effectiveTurbulence * 1.4,
    -22,
    12,
  )
  const pitchDeg = approach(
    currentState.pitchDeg,
    targetPitchDeg,
    input.deltaSeconds * 40,
  )
  const headingRad = toRadians(headingDeg)
  const windHeadingRad = toRadians(atmosphere.windHeadingDeg)
  const airspeedMetersPerSecond = airspeedKmh / 3.6
  const windMetersPerSecond = atmosphere.windSpeedKmh / 3.6
  const eastMetersPerSecond =
    Math.sin(headingRad) * airspeedMetersPerSecond +
    Math.sin(windHeadingRad) * windMetersPerSecond
  const northMetersPerSecond =
    Math.cos(headingRad) * airspeedMetersPerSecond +
    Math.cos(windHeadingRad) * windMetersPerSecond
  const groundSpeedKmh = Math.hypot(eastMetersPerSecond, northMetersPerSecond) * 3.6
  const nextLatitude =
    currentState.latitude +
    (northMetersPerSecond * input.deltaSeconds) / METERS_PER_LATITUDE_DEGREE
  const nextLongitude =
    currentState.longitude +
    (eastMetersPerSecond * input.deltaSeconds) /
      getMetersPerLongitudeDegree(currentState.latitude)
  const unclampedAltitudeMeters =
    currentState.altitudeMeters + verticalSpeedMetersPerSecond * input.deltaSeconds
  const landingMetrics = getLandingZoneMetrics(
    nextLatitude,
    nextLongitude,
    headingDeg,
    input.site.landingZone,
  )

  if (unclampedAltitudeMeters <= terrainHeightMeters + 0.4) {
    const touchdownVerticalSpeedMetersPerSecond =
      verticalSpeedMetersPerSecond *
      (1 - clamp(flareEffectiveness * 0.72, 0, 0.72))
    const touchdownGroundSpeedKmh =
      groundSpeedKmh * (1 - clamp(flareEffectiveness * 0.42, 0, 0.42))
    const landingRating = getLandingRating(
      touchdownVerticalSpeedMetersPerSecond,
      touchdownGroundSpeedKmh,
      bankDeg,
      landingMetrics,
    )

    return {
      latitude: nextLatitude,
      longitude: nextLongitude,
      altitudeMeters: terrainHeightMeters,
      terrainHeightMeters,
      headingDeg,
      bankDeg: landingRating === 'smooth' || landingRating === 'firm' ? 0 : bankDeg,
      pitchDeg: landingRating === 'smooth' ? 0 : pitchDeg,
      airspeedKmh: 0,
      groundSpeedKmh: 0,
      verticalSpeedMetersPerSecond: 0,
      ridgeLiftMetersPerSecond: ridgeAirMass.ridgeLiftMetersPerSecond,
      thermalLiftMetersPerSecond: thermalAirMass.liftMetersPerSecond,
      airMassSinkMetersPerSecond,
      groundClearanceMeters: 0,
      distanceKm:
        currentState.distanceKm +
        (groundSpeedKmh * input.deltaSeconds) / 3_600,
      elapsedSeconds: currentState.elapsedSeconds + input.deltaSeconds,
      turnRateDegPerSecond: 0,
      stallWarning,
      flareEffectiveness,
      landingZoneDistanceMeters: landingMetrics?.distanceMeters ?? null,
      landingApproachErrorDeg: landingMetrics?.approachErrorDeg ?? null,
      landingRating,
      flightPhase: landingRating === 'crash' ? 'crashed' : 'landed',
    }
  }

  const altitudeMeters = unclampedAltitudeMeters
  const groundClearanceMeters = altitudeMeters - terrainHeightMeters
  let flightPhase: FlightPhase = currentState.elapsedSeconds < 8 ? 'launch' : 'soaring'

  if (groundClearanceMeters < 24) {
    flightPhase = 'approach'
  }

  if (flareEffectiveness > 0.1 && groundClearanceMeters < 10) {
    flightPhase = 'flare'
  }

  return {
    latitude: nextLatitude,
    longitude: nextLongitude,
    altitudeMeters,
    terrainHeightMeters,
    headingDeg,
    bankDeg,
    pitchDeg,
    airspeedKmh,
    groundSpeedKmh,
    verticalSpeedMetersPerSecond,
    ridgeLiftMetersPerSecond: ridgeAirMass.ridgeLiftMetersPerSecond,
    thermalLiftMetersPerSecond: thermalAirMass.liftMetersPerSecond,
    airMassSinkMetersPerSecond,
    groundClearanceMeters,
    distanceKm:
      currentState.distanceKm +
      (groundSpeedKmh * input.deltaSeconds) / 3_600,
    elapsedSeconds: currentState.elapsedSeconds + input.deltaSeconds,
    turnRateDegPerSecond,
    stallWarning,
    flareEffectiveness,
    landingZoneDistanceMeters: landingMetrics?.distanceMeters ?? null,
    landingApproachErrorDeg: landingMetrics?.approachErrorDeg ?? null,
    landingRating: 'none',
    flightPhase,
  }
}

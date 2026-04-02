import type { ActivityMode, ActivityModeId } from '../data'
import type { CountryId } from './typing-content'
import type { TypingInputMode } from './typing-engine'

export type WorldSessionState = {
  activityModeId: ActivityModeId
  countryId: CountryId
  elapsedMs: number
  travelProgress: number
  distanceKm: number
  windHeading: number
  windSpeedKmh: number
  turbulence: number
  liftCharge: number
}

export type WorldTickInput = {
  deltaMs: number
  activityMode: ActivityMode
  countryId: CountryId
  accuracy: number
  streak: number
  mistakes: number
}

export type WorldDerivedMetrics = {
  speedKmh: number
  windSpeedKmh: number
  windHeading: number
  turbulence: number
  turbulencePercent: number
  liftCharge: number
  liftPercent: number
  travelProgress: number
  travelPercent: number
  distanceKm: number
  altitudeMeters: number
  moodLabel: string
  windLabel: string
}

function getCountrySeed(countryId: CountryId) {
  return countryId
    .split('')
    .reduce((total, character, index) => total + character.charCodeAt(0) * (index + 1), 0)
}

function clamp(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value))
}

function getRouteLengthKm(activityModeId: ActivityModeId) {
  switch (activityModeId) {
    case 'paragliding':
      return 54
    case 'scenic-flight':
      return 42
    case 'boat-tour':
      return 28
    case 'car-ride':
      return 18
    default:
      return 36
  }
}

function getBaseAltitudeMeters(activityModeId: ActivityModeId) {
  switch (activityModeId) {
    case 'paragliding':
      return 980
    case 'scenic-flight':
      return 420
    case 'boat-tour':
      return 12
    case 'car-ride':
      return 34
    default:
      return 100
  }
}

export function createInitialWorldSession(
  activityModeId: ActivityModeId,
  countryId: CountryId,
): WorldSessionState {
  return {
    activityModeId,
    countryId,
    elapsedMs: 0,
    travelProgress: 0,
    distanceKm: 0,
    windHeading: 18,
    windSpeedKmh: 16,
    turbulence: activityModeId === 'paragliding' ? 0.22 : 0.08,
    liftCharge: activityModeId === 'paragliding' ? 0.48 : 0.2,
  }
}

export function resetWorldSession(
  state: WorldSessionState,
  activityModeId: ActivityModeId = state.activityModeId,
  countryId: CountryId = state.countryId,
) {
  return createInitialWorldSession(activityModeId, countryId)
}

export function selectWorldActivity(
  state: WorldSessionState,
  activityModeId: ActivityModeId,
) {
  return createInitialWorldSession(activityModeId, state.countryId)
}

export function selectWorldCountry(
  state: WorldSessionState,
  countryId: CountryId,
) {
  return createInitialWorldSession(state.activityModeId, countryId)
}

export function tickWorldSession(
  state: WorldSessionState,
  input: WorldTickInput,
): WorldSessionState {
  const nextElapsed = state.elapsedMs + input.deltaMs
  const countrySeed = getCountrySeed(input.countryId) / 100
  const phase = nextElapsed / 7000 + countrySeed
  const windSpeedKmh = clamp(
    18 +
      Math.sin(phase * 0.9) * 7 +
      Math.cos(phase * 0.43) * 4 +
      input.activityMode.windExposure * 6,
    4,
    42,
  )
  const windHeading = (nextElapsed / 180 + countrySeed * 27) % 360
  const windPressure = windSpeedKmh / 42
  const turbulence = clamp(
    input.activityMode.turbulenceFloor +
      windPressure * input.activityMode.windExposure * 0.55 +
      Math.abs(Math.sin(phase * 1.8)) * 0.18 +
      input.mistakes * 0.008 -
      input.streak * 0.004,
    0.04,
    0.98,
  )

  const typingDiscipline = clamp(
    input.accuracy / 100 + input.streak / 30 - input.mistakes * 0.02,
    0.1,
    1.4,
  )

  const liftCharge = input.activityMode.isActiveFlight
    ? clamp(
        0.2 +
          typingDiscipline * input.activityMode.typingInfluence * 0.58 -
          turbulence * 0.16,
        0.08,
        1,
      )
    : clamp(
        0.24 +
          input.activityMode.autopilotStrength * 0.35 +
          Math.sin(phase * 0.75) * 0.06,
        0.12,
        0.72,
      )

  const tailwindFactor = (Math.cos((windHeading * Math.PI) / 180) + 1) / 2
  const speedKmh = clamp(
    input.activityMode.baseSpeedKmh +
      input.activityMode.maxBoostSpeedKmh * liftCharge +
      tailwindFactor * 8 -
      turbulence * 9,
    6,
    input.activityMode.baseSpeedKmh + input.activityMode.maxBoostSpeedKmh + 18,
  )

  const traveledKm = speedKmh * (input.deltaMs / 3_600_000)
  const routeLengthKm = getRouteLengthKm(input.activityMode.id)
  const travelProgress = (state.travelProgress + traveledKm / routeLengthKm) % 1

  return {
    activityModeId: input.activityMode.id,
    countryId: input.countryId,
    elapsedMs: nextElapsed,
    travelProgress,
    distanceKm: state.distanceKm + traveledKm,
    windHeading,
    windSpeedKmh,
    turbulence,
    liftCharge,
  }
}

export function deriveWorldMetrics(
  state: WorldSessionState,
  activityMode: ActivityMode,
): WorldDerivedMetrics {
  const tailwindFactor = (Math.cos((state.windHeading * Math.PI) / 180) + 1) / 2
  const speedKmh = clamp(
    activityMode.baseSpeedKmh +
      activityMode.maxBoostSpeedKmh * state.liftCharge +
      tailwindFactor * 8 -
      state.turbulence * 9,
    6,
    activityMode.baseSpeedKmh + activityMode.maxBoostSpeedKmh + 18,
  )
  const turbulencePercent = Math.round(state.turbulence * 100)
  const liftPercent = Math.round(state.liftCharge * 100)
  const travelPercent = Math.round(state.travelProgress * 100)
  const altitudeMeters = Math.round(
    getBaseAltitudeMeters(activityMode.id) +
      state.liftCharge * (activityMode.isActiveFlight ? 920 : 140) +
      Math.sin(state.elapsedMs / 3400) * (activityMode.isActiveFlight ? 44 : 8),
  )

  const moodLabel = activityMode.isActiveFlight
    ? state.turbulence > 0.58
      ? 'rough air'
      : state.liftCharge > 0.68
        ? 'clean lift'
        : 'working the wind'
    : state.windSpeedKmh > 24
      ? 'fast sightseeing'
      : 'easy cruise'

  const windLabel =
    state.windSpeedKmh > 28
      ? 'crosswind'
      : state.windSpeedKmh > 18
        ? 'steady wind'
        : 'soft air'

  return {
    speedKmh: Math.round(speedKmh),
    windSpeedKmh: Math.round(state.windSpeedKmh),
    windHeading: Math.round(state.windHeading),
    turbulence: state.turbulence,
    turbulencePercent,
    liftCharge: state.liftCharge,
    liftPercent,
    travelProgress: state.travelProgress,
    travelPercent,
    distanceKm: Number(state.distanceKm.toFixed(1)),
    altitudeMeters,
    moodLabel,
    windLabel,
  }
}

export function deriveWorldFlightStyle(
  state: WorldSessionState,
  activityMode: ActivityMode,
  lastInput: TypingInputMode,
  streak: number,
) {
  const metrics = deriveWorldMetrics(state, activityMode)
  const altitudeBand = clamp(metrics.altitudeMeters / 2200, 0.08, 1)
  const gliderTop = activityMode.isActiveFlight
    ? `${62 - altitudeBand * 26 - state.turbulence * 7}%`
    : `${66 - state.travelProgress * 8}%`
  const gliderTilt =
    lastInput === 'wrong'
      ? '12deg'
      : `${Math.max(-14, 3 - streak * 0.28 + Math.sin(state.elapsedMs / 1600) * 4)}deg`

  return {
    gliderLeft: `${14 + state.travelProgress * 70}%`,
    gliderTop,
    gliderTilt,
    routeProgress: `${state.travelProgress}`,
  }
}

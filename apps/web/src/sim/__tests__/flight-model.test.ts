import { describe, expect, it } from 'vitest'
import {
  type AmbientAirState,
  createInitialFlightState,
  resetFlightStateForSite,
  stepFlightState,
  type FlightSimState,
} from '../flight-model'
import {
  DEFAULT_HOME_ROW_CONTROLS,
  stepHomeRowControls,
  type HomeRowControlState,
} from '../home-row-controls'
import type { FlightSite } from '../site-data'

const testSite: FlightSite = {
  id: 'test-site',
  name: 'Test Site',
  countryId: 'italy',
  region: 'Test Range',
  country: 'Testland',
  description: 'Deterministic fixture used to exercise the flight model.',
  latitude: 46,
  longitude: 7,
  launchAltitudeMeters: 1000,
  spawnAglMeters: 120,
  prevailingWindHeadingDeg: 0,
  windSpeedKmh: 0,
  baseRidgeLiftMetersPerSecond: 0,
  routeLengthKm: 10,
  thermals: [],
}

const thermalSite: FlightSite = {
  ...testSite,
  thermals: [
    {
      id: 'core',
      latitude: 46,
      longitude: 7,
      radiusMeters: 1500,
      liftMetersPerSecond: 2.5,
    },
  ],
}

const ridgeSite: FlightSite = {
  ...testSite,
  ridge: {
    latitude: testSite.latitude,
    longitude: testSite.longitude,
    axisHeadingDeg: 0,
    lengthMeters: 2200,
    windwardDepthMeters: 420,
    leeDepthMeters: 520,
    peakLiftMetersPerSecond: 1.8,
    leeSinkMetersPerSecond: 1,
  },
}

const landingSite: FlightSite = {
  ...testSite,
  landingZone: {
    latitude: testSite.latitude,
    longitude: testSite.longitude,
    headingDeg: 0,
    lengthMeters: 240,
    widthMeters: 80,
  },
}

function makeStepInput(
  overrides: Partial<{
    deltaSeconds: number
    controls: HomeRowControlState
    site: FlightSite
    terrainHeightMeters: number | null
    atmosphere: AmbientAirState | null
  }> = {},
) {
  return {
    deltaSeconds: 0.2,
    controls: DEFAULT_HOME_ROW_CONTROLS,
    site: testSite,
    terrainHeightMeters: testSite.launchAltitudeMeters,
    atmosphere: null,
    ...overrides,
  }
}

function makeState(overrides: Partial<FlightSimState> = {}): FlightSimState {
  return {
    latitude: testSite.latitude,
    longitude: testSite.longitude,
    altitudeMeters: 1120,
    terrainHeightMeters: testSite.launchAltitudeMeters,
    headingDeg: 0,
    bankDeg: 0,
    pitchDeg: -6,
    airspeedKmh: 36,
    groundSpeedKmh: 36,
    verticalSpeedMetersPerSecond: 0,
    ridgeLiftMetersPerSecond: 0,
    thermalLiftMetersPerSecond: 0,
    groundClearanceMeters: 120,
    distanceKm: 0,
    elapsedSeconds: 0,
    turnRateDegPerSecond: 0,
    stallWarning: 0,
    flareEffectiveness: 0,
    landingZoneDistanceMeters: null,
    landingApproachErrorDeg: null,
    landingRating: 'none',
    flightPhase: 'soaring',
    ...overrides,
  }
}

function headingDeltaDegrees(from: number, to: number) {
  return ((to - from + 540) % 360) - 180
}

function buildControls(
  overrides: Partial<HomeRowControlState> = {},
  steps = 1,
  deltaSeconds = 0.2,
) {
  let controls: HomeRowControlState = {
    ...DEFAULT_HOME_ROW_CONTROLS,
    ...overrides,
  }

  for (let index = 0; index < steps; index += 1) {
    controls = stepHomeRowControls(controls, deltaSeconds)
  }

  return controls
}

describe('createInitialFlightState', () => {
  it('seeds position, altitude, and clearance from the site and terrain', () => {
    const state = createInitialFlightState(testSite, 1200)

    expect(state.latitude).toBe(testSite.latitude)
    expect(state.longitude).toBe(testSite.longitude)
    expect(state.altitudeMeters).toBe(1320)
    expect(state.terrainHeightMeters).toBe(1200)
    expect(state.groundClearanceMeters).toBe(testSite.spawnAglMeters)
    expect(state.headingDeg).toBe(testSite.prevailingWindHeadingDeg)
  })
})

describe('resetFlightStateForSite', () => {
  it('falls back to launch altitude when no terrain sample is available', () => {
    const reset = resetFlightStateForSite(testSite, null)
    const initial = createInitialFlightState(testSite, testSite.launchAltitudeMeters)

    expect(reset).toEqual(initial)
  })
})

describe('stepFlightState', () => {
  it('is deterministic for the same state and input', () => {
    const state = createInitialFlightState(testSite)
    const input = makeStepInput()

    expect(stepFlightState(state, input)).toEqual(stepFlightState(state, input))
  })

  it('builds a held brake into a progressive turn instead of snapping to full bank', () => {
    const first = stepFlightState(
      createInitialFlightState(testSite),
      makeStepInput({
        controls: buildControls({ leftBrake: true }, 1),
      }),
    )
    const second = stepFlightState(
      first,
      makeStepInput({
        controls: buildControls({ leftBrake: true }, 2),
      }),
    )
    const third = stepFlightState(
      second,
      makeStepInput({
        controls: buildControls({ leftBrake: true }, 3),
      }),
    )

    expect(first.bankDeg).toBeLessThan(0)
    expect(Math.abs(first.bankDeg)).toBeLessThan(Math.abs(second.bankDeg))
    expect(Math.abs(second.bankDeg)).toBeLessThanOrEqual(Math.abs(third.bankDeg))
    expect(third.bankDeg).toBeLessThan(-14)
    expect(third.bankDeg).toBeGreaterThan(-30)
    expect(headingDeltaDegrees(0, first.headingDeg)).toBeLessThan(0)
    expect(headingDeltaDegrees(first.headingDeg, second.headingDeg)).toBeLessThan(0)
    expect(headingDeltaDegrees(second.headingDeg, third.headingDeg)).toBeLessThan(0)
  })

  it('slows down and sinks more when both brakes are held together', () => {
    const neutral = stepFlightState(createInitialFlightState(testSite), makeStepInput())
    const symmetricBrake = stepFlightState(
      createInitialFlightState(testSite),
      makeStepInput({
        controls: buildControls({
          leftBrake: true,
          rightBrake: true,
        }),
      }),
    )

    expect(symmetricBrake.bankDeg).toBeCloseTo(0, 4)
    expect(symmetricBrake.airspeedKmh).toBeLessThan(neutral.airspeedKmh)
    expect(symmetricBrake.verticalSpeedMetersPerSecond).toBeLessThan(
      neutral.verticalSpeedMetersPerSecond,
    )
  })

  it('lets weight shift alone roll the wing and add sink without brake input', () => {
    const neutral = stepFlightState(createInitialFlightState(testSite), makeStepInput())
    const weightShift = stepFlightState(
      createInitialFlightState(testSite),
      makeStepInput({
        controls: buildControls({
          weightRight: true,
        }),
      }),
    )

    expect(weightShift.bankDeg).toBeGreaterThan(0)
    expect(weightShift.verticalSpeedMetersPerSecond).toBeLessThan(
      neutral.verticalSpeedMetersPerSecond,
    )
  })

  it('trades speed for sink when the speed bar is pushed', () => {
    const calm = stepFlightState(createInitialFlightState(testSite), makeStepInput())
    const boosted = stepFlightState(
      createInitialFlightState(testSite),
      makeStepInput({
        controls: buildControls({
          speedBar: true,
        }),
      }),
    )

    expect(boosted.airspeedKmh).toBeGreaterThan(calm.airspeedKmh)
    expect(boosted.pitchDeg).toBeGreaterThan(calm.pitchDeg)
    expect(boosted.verticalSpeedMetersPerSecond).toBeLessThan(calm.verticalSpeedMetersPerSecond)
  })

  it('keeps the glider above the terrain floor on a bad approach', () => {
    const next = stepFlightState(
      makeState({
        altitudeMeters: 50,
        terrainHeightMeters: 500,
        groundClearanceMeters: -450,
      }),
      makeStepInput({
        terrainHeightMeters: 500,
      }),
    )

    expect(next.altitudeMeters).toBe(500)
    expect(next.groundClearanceMeters).toBe(0)
    expect(next.flightPhase === 'landed' || next.flightPhase === 'crashed').toBe(true)
  })

  it('adds thermal lift when the wing passes through a seeded thermal', () => {
    const empty = stepFlightState(createInitialFlightState(testSite), makeStepInput())
    const thermal = stepFlightState(
      createInitialFlightState(thermalSite),
      makeStepInput({ site: thermalSite }),
    )

    expect(thermal.thermalLiftMetersPerSecond).toBeGreaterThan(
      empty.thermalLiftMetersPerSecond,
    )
    expect(thermal.verticalSpeedMetersPerSecond).toBeGreaterThan(
      empty.verticalSpeedMetersPerSecond,
    )
  })

  it('rewards the windward side of a ridge and punishes the lee side', () => {
    const atmosphere = {
      windHeadingDeg: 90,
      windSpeedKmh: 24,
      turbulence: 0,
    }
    const windward = stepFlightState(
      makeState({
        longitude:
          ridgeSite.longitude -
          280 / 77_000,
      }),
      makeStepInput({
        site: ridgeSite,
        atmosphere,
      }),
    )
    const lee = stepFlightState(
      makeState({
        longitude:
          ridgeSite.longitude +
          280 / 77_000,
      }),
      makeStepInput({
        site: ridgeSite,
        atmosphere,
      }),
    )

    expect(windward.ridgeLiftMetersPerSecond).toBeGreaterThan(0)
    expect(lee.airMassSinkMetersPerSecond).toBeGreaterThan(0)
    expect(windward.verticalSpeedMetersPerSecond).toBeGreaterThan(
      lee.verticalSpeedMetersPerSecond,
    )
  })

  it('uses flare timing to improve touchdown quality near the landing zone', () => {
    const noFlare = stepFlightState(
      makeState({
        altitudeMeters: 1000.35,
        terrainHeightMeters: 1000,
        groundClearanceMeters: 0.35,
        airspeedKmh: 28,
        groundSpeedKmh: 28,
        elapsedSeconds: 36,
        flightPhase: 'approach',
      }),
      makeStepInput({
        site: landingSite,
      }),
    )
    const flared = stepFlightState(
      makeState({
        altitudeMeters: 1000.35,
        terrainHeightMeters: 1000,
        groundClearanceMeters: 0.35,
        airspeedKmh: 28,
        groundSpeedKmh: 28,
        elapsedSeconds: 36,
        flightPhase: 'approach',
      }),
      makeStepInput({
        site: landingSite,
        controls: buildControls({
          leftBrake: true,
          rightBrake: true,
        }, 2),
      }),
    )

    expect(flared.flareEffectiveness).toBeGreaterThan(noFlare.flareEffectiveness)
    expect(flared.flightPhase === 'landed' || flared.flightPhase === 'crashed').toBe(true)
    expect(flared.landingRating).not.toBe('none')
    expect(flared.landingRating).not.toBe('crash')
  })
})

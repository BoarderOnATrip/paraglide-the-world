import { describe, it, expect } from 'vitest'
import {
  createInitialWorldSession,
  tickWorldSession,
  deriveWorldMetrics,
  type WorldSessionState,
  type WorldTickInput,
} from '../world-engine'
import { ACTIVITY_MODES } from '../../data'

const paragliding = ACTIVITY_MODES.find((m) => m.id === 'paragliding')!
const scenic = ACTIVITY_MODES.find((m) => m.id === 'scenic-flight')!

function makeTick(overrides: Partial<WorldTickInput> = {}): WorldTickInput {
  return {
    deltaMs: 120,
    activityMode: paragliding,
    countryId: 'brazil',
    accuracy: 85,
    streak: 5,
    mistakes: 3,
    ...overrides,
  }
}

// ---------------------------------------------------------------------------
// createInitialWorldSession
// ---------------------------------------------------------------------------

describe('createInitialWorldSession', () => {
  it('gives paragliding higher initial turbulence and lift than scenic', () => {
    const pg = createInitialWorldSession('paragliding', 'brazil')
    const sc = createInitialWorldSession('scenic-flight', 'brazil')
    expect(pg.turbulence).toBeGreaterThan(sc.turbulence)
    expect(pg.liftCharge).toBeGreaterThan(sc.liftCharge)
  })

  it('starts at zero elapsed time, travel, and distance', () => {
    const s = createInitialWorldSession('paragliding', 'brazil')
    expect(s.elapsedMs).toBe(0)
    expect(s.travelProgress).toBe(0)
    expect(s.distanceKm).toBe(0)
  })
})

// ---------------------------------------------------------------------------
// tickWorldSession
// ---------------------------------------------------------------------------

describe('tickWorldSession', () => {
  it('is deterministic: same state + input produces identical output', () => {
    const state = createInitialWorldSession('paragliding', 'brazil')
    const input = makeTick()
    const a = tickWorldSession(state, input)
    const b = tickWorldSession(state, input)
    expect(a).toEqual(b)
  })

  it('advances travelProgress forward', () => {
    const state = createInitialWorldSession('paragliding', 'brazil')
    const next = tickWorldSession(state, makeTick())
    expect(next.travelProgress).toBeGreaterThan(0)
  })

  it('wraps travelProgress via modulo when it exceeds 1', () => {
    const state: WorldSessionState = {
      activityModeId: 'paragliding',
      countryId: 'brazil',
      elapsedMs: 50_000,
      travelProgress: 0.999,
      distanceKm: 53,
      windHeading: 90,
      windSpeedKmh: 20,
      turbulence: 0.2,
      liftCharge: 0.7,
    }
    const next = tickWorldSession(state, makeTick({ deltaMs: 60_000 }))
    expect(next.travelProgress).toBeGreaterThanOrEqual(0)
    expect(next.travelProgress).toBeLessThan(1)
  })

  it('accumulates elapsed time and distance', () => {
    const state = createInitialWorldSession('paragliding', 'brazil')
    const next = tickWorldSession(state, makeTick({ deltaMs: 120 }))
    expect(next.elapsedMs).toBe(120)
    expect(next.distanceKm).toBeGreaterThan(0)
  })

  it('clamps turbulence to upper bound with extreme mistakes', () => {
    const state = createInitialWorldSession('paragliding', 'brazil')
    const next = tickWorldSession(state, makeTick({ mistakes: 200, streak: 0 }))
    expect(next.turbulence).toBe(0.98)
  })

  it('clamps turbulence to lower bound with extreme streak', () => {
    const state = createInitialWorldSession('paragliding', 'brazil')
    const next = tickWorldSession(state, makeTick({ mistakes: 0, streak: 300 }))
    expect(next.turbulence).toBe(0.04)
  })

  it('active flight lift responds to typing accuracy', () => {
    const state = createInitialWorldSession('paragliding', 'brazil')
    const good = tickWorldSession(
      state,
      makeTick({ accuracy: 100, streak: 30, mistakes: 0 }),
    )
    const bad = tickWorldSession(
      state,
      makeTick({ accuracy: 20, streak: 0, mistakes: 50 }),
    )
    expect(good.liftCharge).toBeGreaterThan(bad.liftCharge)
  })

  it('passive flight lift ignores accuracy', () => {
    const state = createInitialWorldSession('scenic-flight', 'brazil')
    const good = tickWorldSession(
      state,
      makeTick({ activityMode: scenic, accuracy: 100, streak: 30, mistakes: 0 }),
    )
    const bad = tickWorldSession(
      state,
      makeTick({ activityMode: scenic, accuracy: 20, streak: 0, mistakes: 50 }),
    )
    expect(good.liftCharge).toBe(bad.liftCharge)
  })

  it('produces different wind patterns for different countries', () => {
    const br = tickWorldSession(
      createInitialWorldSession('paragliding', 'brazil'),
      makeTick({ countryId: 'brazil' }),
    )
    const jp = tickWorldSession(
      createInitialWorldSession('paragliding', 'japan'),
      makeTick({ countryId: 'japan' }),
    )
    expect(br.windSpeedKmh).not.toBe(jp.windSpeedKmh)
  })
})

// ---------------------------------------------------------------------------
// deriveWorldMetrics
// ---------------------------------------------------------------------------

describe('deriveWorldMetrics', () => {
  function makeState(overrides: Partial<WorldSessionState> = {}): WorldSessionState {
    return {
      activityModeId: 'paragliding',
      countryId: 'brazil',
      elapsedMs: 10_000,
      travelProgress: 0.3,
      distanceKm: 15,
      windHeading: 90,
      windSpeedKmh: 25,
      turbulence: 0.3,
      liftCharge: 0.5,
      ...overrides,
    }
  }

  it('labels "rough air" when turbulence > 0.58 in active flight', () => {
    const m = deriveWorldMetrics(makeState({ turbulence: 0.7 }), paragliding)
    expect(m.moodLabel).toBe('rough air')
  })

  it('labels "clean lift" when lift > 0.68 and turbulence low in active flight', () => {
    const m = deriveWorldMetrics(
      makeState({ turbulence: 0.3, liftCharge: 0.75 }),
      paragliding,
    )
    expect(m.moodLabel).toBe('clean lift')
  })

  it('labels "working the wind" for moderate active conditions', () => {
    const m = deriveWorldMetrics(
      makeState({ turbulence: 0.4, liftCharge: 0.5 }),
      paragliding,
    )
    expect(m.moodLabel).toBe('working the wind')
  })

  it('uses passive mood labels based on wind speed', () => {
    const fast = deriveWorldMetrics(makeState({ windSpeedKmh: 26 }), scenic)
    expect(fast.moodLabel).toBe('fast sightseeing')

    const easy = deriveWorldMetrics(makeState({ windSpeedKmh: 20 }), scenic)
    expect(easy.moodLabel).toBe('easy cruise')
  })

  it('labels wind by speed thresholds', () => {
    expect(deriveWorldMetrics(makeState({ windSpeedKmh: 30 }), paragliding).windLabel)
      .toBe('crosswind')
    expect(deriveWorldMetrics(makeState({ windSpeedKmh: 22 }), paragliding).windLabel)
      .toBe('steady wind')
    expect(deriveWorldMetrics(makeState({ windSpeedKmh: 12 }), paragliding).windLabel)
      .toBe('soft air')
  })

  it('paragliding base altitude is higher than scenic', () => {
    const state = makeState({ liftCharge: 0.5 })
    const pg = deriveWorldMetrics(state, paragliding).altitudeMeters
    const sc = deriveWorldMetrics(
      { ...state, activityModeId: 'scenic-flight' },
      scenic,
    ).altitudeMeters
    expect(pg).toBeGreaterThan(sc)
  })
})

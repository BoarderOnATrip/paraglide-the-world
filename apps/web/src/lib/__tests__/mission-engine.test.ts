import { describe, it, expect } from 'vitest'
import {
  createInitialMissionSession,
  advanceMissionSession,
  deriveMissionView,
  type MissionAdvanceInput,
} from '../mission-engine'
import { ACTIVITY_MODES } from '../../data'
import { getCountryContent } from '../typing-content'

const paragliding = ACTIVITY_MODES.find((m) => m.id === 'paragliding')!
const scenic = ACTIVITY_MODES.find((m) => m.id === 'scenic-flight')!
const brazil = getCountryContent('brazil')

function makeAdvance(overrides: Partial<MissionAdvanceInput> = {}): MissionAdvanceInput {
  return {
    activityMode: paragliding,
    country: brazil,
    progress: 0,
    distanceKm: 10,
    accuracy: 85,
    mistakes: 3,
    ...overrides,
  }
}

// ---------------------------------------------------------------------------
// createInitialMissionSession
// ---------------------------------------------------------------------------

describe('createInitialMissionSession', () => {
  it('pre-clears the Launch checkpoint', () => {
    const s = createInitialMissionSession('paragliding', 'brazil')
    expect(s.clearedCheckpointIds).toContain('brazil-launch')
    expect(s.clearedCheckpointIds).toHaveLength(1)
  })

  it('starts at run 0 with no result', () => {
    const s = createInitialMissionSession('paragliding', 'brazil')
    expect(s.runCount).toBe(0)
    expect(s.lastResult).toBeNull()
    expect(s.previousProgress).toBe(0)
  })
})

// ---------------------------------------------------------------------------
// advanceMissionSession
// ---------------------------------------------------------------------------

describe('advanceMissionSession', () => {
  it('clears a checkpoint when progress crosses its threshold', () => {
    const s = createInitialMissionSession('paragliding', 'brazil')
    // Brazil: Sugarloaf at 1/3 ≈ 0.333
    const next = advanceMissionSession(s, makeAdvance({ progress: 0.4 }))
    expect(next.clearedCheckpointIds).toContain('brazil-sugarloaf')
  })

  it('does not clear a checkpoint when progress stays below', () => {
    const s = createInitialMissionSession('paragliding', 'brazil')
    const next = advanceMissionSession(s, makeAdvance({ progress: 0.2 }))
    expect(next.clearedCheckpointIds).not.toContain('brazil-sugarloaf')
  })

  it('clears multiple checkpoints in a single advance', () => {
    const s = createInitialMissionSession('paragliding', 'brazil')
    // Progress 0.8 crosses Sugarloaf (0.333) and Forest Lift (0.667)
    const next = advanceMissionSession(s, makeAdvance({ progress: 0.8 }))
    expect(next.clearedCheckpointIds).toContain('brazil-sugarloaf')
    expect(next.clearedCheckpointIds).toContain('brazil-forest-lift')
  })

  it('detects loop completion when progress wraps below previous', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.9 }))
    const next = advanceMissionSession(s, makeAdvance({ progress: 0.05 }))
    expect(next.runCount).toBe(1)
    expect(next.lastResult).not.toBeNull()
    expect(next.lastResult!.runNumber).toBe(1)
  })

  it('resets cleared checkpoints on loop completion', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.9 }))
    const next = advanceMissionSession(s, makeAdvance({ progress: 0.05 }))
    expect(next.clearedCheckpointIds).toEqual(['brazil-launch'])
  })

  it('returns the same reference when nothing changes', () => {
    const s = createInitialMissionSession('paragliding', 'brazil')
    const next = advanceMissionSession(s, makeAdvance({ progress: 0 }))
    expect(next).toBe(s) // same object identity
  })

  // --- Rating ---

  it('awards gold for active flight with high accuracy and few mistakes', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.9 }))
    const next = advanceMissionSession(
      s,
      makeAdvance({ progress: 0.05, accuracy: 98, mistakes: 1 }),
    )
    expect(next.lastResult!.rating).toBe('gold')
  })

  it('awards silver for good-but-not-gold performance', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.9 }))
    const next = advanceMissionSession(
      s,
      makeAdvance({ progress: 0.05, accuracy: 92, mistakes: 4 }),
    )
    expect(next.lastResult!.rating).toBe('silver')
  })

  it('awards bronze for low accuracy', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.9 }))
    const next = advanceMissionSession(
      s,
      makeAdvance({ progress: 0.05, accuracy: 70, mistakes: 15 }),
    )
    expect(next.lastResult!.rating).toBe('bronze')
  })

  it('does not award gold in passive flight regardless of accuracy', () => {
    let s = createInitialMissionSession('scenic-flight', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ activityMode: scenic, progress: 0.9 }))
    const next = advanceMissionSession(
      s,
      makeAdvance({ activityMode: scenic, progress: 0.05, accuracy: 99, mistakes: 0 }),
    )
    expect(next.lastResult!.rating).not.toBe('gold')
  })
})

// ---------------------------------------------------------------------------
// deriveMissionView
// ---------------------------------------------------------------------------

describe('deriveMissionView', () => {
  it('assigns cleared / active / upcoming statuses correctly', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.4 }))
    const view = deriveMissionView(s, brazil, paragliding, 85, 3)
    const statuses = view.checkpoints.map((c) => [c.label, c.status])
    expect(statuses).toEqual([
      ['Launch', 'cleared'],
      ['Sugarloaf', 'cleared'],
      ['Forest Lift', 'active'],
      ['Lagoon', 'upcoming'],
    ])
  })

  it('never marks landing as cleared', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.8 }))
    const view = deriveMissionView(s, brazil, paragliding, 85, 3)
    const landing = view.checkpoints.find((c) => c.kind === 'landing')!
    expect(landing.status).not.toBe('cleared')
  })

  it('identifies the first non-cleared checkpoint as active', () => {
    const s = createInitialMissionSession('paragliding', 'brazil')
    const view = deriveMissionView(s, brazil, paragliding, 85, 3)
    expect(view.activeCheckpoint!.label).toBe('Sugarloaf')
  })

  it('falls back to landing when all intermediates are cleared', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.8 }))
    const view = deriveMissionView(s, brazil, paragliding, 85, 3)
    expect(view.activeCheckpoint!.label).toBe('Lagoon')
    expect(view.activeCheckpoint!.kind).toBe('landing')
  })

  it('counts cleared checkpoints excluding Launch', () => {
    let s = createInitialMissionSession('paragliding', 'brazil')
    s = advanceMissionSession(s, makeAdvance({ progress: 0.4 }))
    const view = deriveMissionView(s, brazil, paragliding, 85, 3)
    expect(view.clearedCount).toBe(1) // Sugarloaf only
    expect(view.totalCount).toBe(3) // Sugarloaf, Forest Lift, Lagoon
  })
})

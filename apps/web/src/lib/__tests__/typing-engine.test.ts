import { describe, it, expect } from 'vitest'
import {
  createInitialTypingSession,
  normalizeTypingKey,
  processTypingKey,
  deriveTypingMetrics,
  getTypingFlightStyleMetrics,
  type TypingSessionState,
} from '../typing-engine'
import { getLessonText, getCountryContent, COUNTRIES } from '../typing-content'

const brazil = getCountryContent('brazil')
const brazilLesson = getLessonText(brazil)

// ---------------------------------------------------------------------------
// normalizeTypingKey
// ---------------------------------------------------------------------------

describe('normalizeTypingKey', () => {
  it('maps "Space" and "Spacebar" to a space character', () => {
    expect(normalizeTypingKey('Space')).toBe(' ')
    expect(normalizeTypingKey('Spacebar')).toBe(' ')
  })

  it('rejects multi-character keys', () => {
    expect(normalizeTypingKey('Shift')).toBeNull()
    expect(normalizeTypingKey('Enter')).toBeNull()
    expect(normalizeTypingKey('Backspace')).toBeNull()
  })

  it('lowercases single characters', () => {
    expect(normalizeTypingKey('A')).toBe('a')
    expect(normalizeTypingKey('z')).toBe('z')
  })

  it('passes through space and digits unchanged', () => {
    expect(normalizeTypingKey(' ')).toBe(' ')
    expect(normalizeTypingKey('5')).toBe('5')
  })
})

// ---------------------------------------------------------------------------
// createInitialTypingSession
// ---------------------------------------------------------------------------

describe('createInitialTypingSession', () => {
  it('starts clean with the first country by default', () => {
    const s = createInitialTypingSession()
    expect(s).toEqual({
      selectedCountryId: COUNTRIES[0].id,
      cursor: 0,
      mistakes: 0,
      streak: 0,
      bestStreak: 0,
      journeys: 0,
      lastInput: 'idle',
    })
  })

  it('accepts a specific country', () => {
    expect(createInitialTypingSession('japan').selectedCountryId).toBe('japan')
  })
})

// ---------------------------------------------------------------------------
// processTypingKey
// ---------------------------------------------------------------------------

describe('processTypingKey', () => {
  it('ignores modifier keys without mutating state', () => {
    const s = createInitialTypingSession('brazil')
    const r = processTypingKey(s, 'Shift')
    expect(r.ignored).toBe(true)
    expect(r.accepted).toBe(false)
    expect(r.state).toBe(s) // same reference
  })

  it('advances cursor and streak on correct key', () => {
    const s = createInitialTypingSession('brazil')
    const r = processTypingKey(s, brazilLesson[0])
    expect(r.correct).toBe(true)
    expect(r.state.cursor).toBe(1)
    expect(r.state.streak).toBe(1)
    expect(r.state.lastInput).toBe('correct')
  })

  it('increments mistakes and resets streak on wrong key — cursor stays', () => {
    const s = createInitialTypingSession('brazil')
    const r = processTypingKey(s, '~')
    expect(r.correct).toBe(false)
    expect(r.state.cursor).toBe(0)
    expect(r.state.mistakes).toBe(1)
    expect(r.state.streak).toBe(0)
    expect(r.state.lastInput).toBe('wrong')
  })

  it('tracks bestStreak through correct and wrong inputs', () => {
    let s = createInitialTypingSession('brazil')
    // 'f','f' correct → streak 2
    s = processTypingKey(s, 'f').state
    s = processTypingKey(s, 'f').state
    expect(s.streak).toBe(2)
    expect(s.bestStreak).toBe(2)

    // wrong key resets streak, bestStreak stays
    s = processTypingKey(s, '~').state
    expect(s.streak).toBe(0)
    expect(s.bestStreak).toBe(2)
  })

  it('resets on lesson completion and increments journeys', () => {
    const s: TypingSessionState = {
      selectedCountryId: 'brazil',
      cursor: brazilLesson.length - 1,
      mistakes: 5,
      streak: 3,
      bestStreak: 10,
      journeys: 2,
      lastInput: 'correct',
    }
    const r = processTypingKey(s, brazilLesson[brazilLesson.length - 1])
    expect(r.completedLesson).toBe(true)
    expect(r.state.cursor).toBe(0)
    expect(r.state.mistakes).toBe(0)
    expect(r.state.streak).toBe(0)
    expect(r.state.journeys).toBe(3)
  })

  it('promotes bestStreak on lesson completion', () => {
    const s: TypingSessionState = {
      selectedCountryId: 'brazil',
      cursor: brazilLesson.length - 1,
      mistakes: 0,
      streak: 15,
      bestStreak: 10,
      journeys: 0,
      lastInput: 'correct',
    }
    const r = processTypingKey(s, brazilLesson[brazilLesson.length - 1])
    // max(10, 15 + 1) = 16
    expect(r.state.bestStreak).toBe(16)
  })

  it('ignores input when cursor is past lesson end', () => {
    const s: TypingSessionState = {
      selectedCountryId: 'brazil',
      cursor: brazilLesson.length,
      mistakes: 0,
      streak: 0,
      bestStreak: 0,
      journeys: 0,
      lastInput: 'idle',
    }
    const r = processTypingKey(s, 'f')
    expect(r.ignored).toBe(true)
    expect(r.state).toBe(s)
  })
})

// ---------------------------------------------------------------------------
// deriveTypingMetrics
// ---------------------------------------------------------------------------

describe('deriveTypingMetrics', () => {
  it('returns 100% accuracy when no keys have been pressed', () => {
    const m = deriveTypingMetrics(createInitialTypingSession('brazil'))
    expect(m.accuracy).toBe(100)
    expect(m.attempts).toBe(0)
  })

  it('calculates accuracy from cursor and mistakes', () => {
    const m = deriveTypingMetrics({
      selectedCountryId: 'brazil',
      cursor: 8,
      mistakes: 2,
      streak: 0,
      bestStreak: 0,
      journeys: 0,
      lastInput: 'correct',
    })
    // attempts = 8 + 2 = 10, accuracy = round(8/10 * 100) = 80
    expect(m.attempts).toBe(10)
    expect(m.accuracy).toBe(80)
  })

  it('computes progress as cursor / lessonLength', () => {
    const m = deriveTypingMetrics({
      selectedCountryId: 'brazil',
      cursor: 19,
      mistakes: 0,
      streak: 0,
      bestStreak: 0,
      journeys: 0,
      lastInput: 'correct',
    })
    expect(m.progress).toBeCloseTo(19 / brazilLesson.length)
  })

  it('clamps activeFactIndex within bounds', () => {
    const start = deriveTypingMetrics(createInitialTypingSession('brazil'))
    expect(start.activeFactIndex).toBe(0)

    const nearEnd = deriveTypingMetrics({
      selectedCountryId: 'brazil',
      cursor: brazilLesson.length - 1,
      mistakes: 0,
      streak: 0,
      bestStreak: 0,
      journeys: 0,
      lastInput: 'correct',
    })
    expect(nearEnd.activeFactIndex).toBeLessThan(brazil.facts.length)
    expect(nearEnd.activeFactIndex).toBeGreaterThanOrEqual(0)
  })

  it('marks isComplete when cursor reaches lesson end', () => {
    const m = deriveTypingMetrics({
      selectedCountryId: 'brazil',
      cursor: brazilLesson.length,
      mistakes: 0,
      streak: 0,
      bestStreak: 0,
      journeys: 0,
      lastInput: 'correct',
    })
    expect(m.isComplete).toBe(true)
  })

  it('is not complete mid-lesson', () => {
    const m = deriveTypingMetrics(createInitialTypingSession('brazil'))
    expect(m.isComplete).toBe(false)
  })
})

// ---------------------------------------------------------------------------
// getTypingFlightStyleMetrics
// ---------------------------------------------------------------------------

describe('getTypingFlightStyleMetrics', () => {
  it('tilts 10deg on wrong input', () => {
    const style = getTypingFlightStyleMetrics({
      selectedCountryId: 'brazil',
      cursor: 5,
      mistakes: 1,
      streak: 0,
      bestStreak: 0,
      journeys: 0,
      lastInput: 'wrong',
    })
    expect(style.gliderTilt).toBe('10deg')
  })

  it('reduces tilt with higher streak', () => {
    const style = getTypingFlightStyleMetrics({
      selectedCountryId: 'brazil',
      cursor: 5,
      mistakes: 0,
      streak: 20,
      bestStreak: 20,
      journeys: 0,
      lastInput: 'correct',
    })
    // max(-10, 2 - 20 * 0.35) = max(-10, -5) = -5
    expect(style.gliderTilt).toBe('-5deg')
  })
})

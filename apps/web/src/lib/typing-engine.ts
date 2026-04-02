import {
  COUNTRIES,
  type CountryContent,
  type CountryId,
  type FingerGuide,
  getCountryContent,
  getFingerGuide,
  getLessonText,
} from './typing-content'

export type TypingInputMode = 'idle' | 'correct' | 'wrong'

export type TypingSessionState = {
  selectedCountryId: CountryId
  cursor: number
  mistakes: number
  streak: number
  bestStreak: number
  journeys: number
  lastInput: TypingInputMode
}

export type TypingDerivedMetrics = {
  country: CountryContent
  lessonText: string
  completedText: string
  currentKey: string
  upcomingText: string
  progress: number
  attempts: number
  accuracy: number
  altitude: number
  glideSpeed: number
  activeFactIndex: number
  activeFact: string
  activeFinger: FingerGuide
  isComplete: boolean
}

export type TypingKeyResult = {
  accepted: boolean
  correct: boolean
  ignored: boolean
  completedLesson: boolean
  expectedKey: string | null
  pressedKey: string | null
  state: TypingSessionState
}

export function createInitialTypingSession(
  countryId: CountryId = COUNTRIES[0].id,
): TypingSessionState {
  return {
    selectedCountryId: countryId,
    cursor: 0,
    mistakes: 0,
    streak: 0,
    bestStreak: 0,
    journeys: 0,
    lastInput: 'idle',
  }
}

export function resetTypingSession(
  state: TypingSessionState,
  countryId: CountryId = state.selectedCountryId,
): TypingSessionState {
  return createInitialTypingSession(countryId)
}

export function selectTypingCountry(
  state: TypingSessionState,
  countryId: CountryId,
): TypingSessionState {
  return resetTypingSession(state, countryId)
}

export function normalizeTypingKey(key: string) {
  if (key === 'Space' || key === 'Spacebar') {
    return ' '
  }

  if (key.length !== 1) {
    return null
  }

  return key.toLowerCase()
}

export function processTypingKey(
  state: TypingSessionState,
  key: string,
): TypingKeyResult {
  const pressedKey = normalizeTypingKey(key)

  if (pressedKey === null) {
    return {
      accepted: false,
      correct: false,
      ignored: true,
      completedLesson: false,
      expectedKey: null,
      pressedKey: null,
      state,
    }
  }

  const country = getCountryContent(state.selectedCountryId)
  const lessonText = getLessonText(country)
  const expectedKey = (lessonText[state.cursor] ?? '').toLowerCase()

  if (!expectedKey) {
    return {
      accepted: false,
      correct: false,
      ignored: true,
      completedLesson: false,
      expectedKey: null,
      pressedKey,
      state,
    }
  }

  if (pressedKey !== expectedKey) {
    return {
      accepted: true,
      correct: false,
      ignored: false,
      completedLesson: false,
      expectedKey,
      pressedKey,
      state: {
        ...state,
        mistakes: state.mistakes + 1,
        streak: 0,
        lastInput: 'wrong',
      },
    }
  }

  const nextCursor = state.cursor + 1
  const nextStreak = state.streak + 1
  const completed = nextCursor >= lessonText.length

  return {
    accepted: true,
    correct: true,
    ignored: false,
    completedLesson: completed,
    expectedKey,
    pressedKey,
    state: completed
      ? {
          ...state,
          cursor: 0,
          mistakes: 0,
          streak: 0,
          bestStreak: Math.max(state.bestStreak, nextStreak),
          journeys: state.journeys + 1,
          lastInput: 'correct',
        }
      : {
          ...state,
          cursor: nextCursor,
          streak: nextStreak,
          bestStreak: Math.max(state.bestStreak, nextStreak),
          lastInput: 'correct',
        },
  }
}

export function deriveTypingMetrics(
  state: TypingSessionState,
): TypingDerivedMetrics {
  const country = getCountryContent(state.selectedCountryId)
  const lessonText = getLessonText(country)
  const completedText = lessonText.slice(Math.max(0, state.cursor - 18), state.cursor)
  const currentKey = lessonText[state.cursor] ?? lessonText[0] ?? ''
  const upcomingText = lessonText.slice(state.cursor + 1, state.cursor + 24)
  const progress = lessonText.length === 0 ? 0 : state.cursor / lessonText.length
  const attempts = state.cursor + state.mistakes
  const accuracy =
    attempts === 0 ? 100 : Math.round((state.cursor / Math.max(attempts, 1)) * 100)
  const altitude = Math.round(720 + progress * 1150 + accuracy * 3 - state.mistakes * 8)
  const glideSpeed = Math.round(24 + accuracy * 0.16 + state.streak * 0.9)
  const activeFactIndex = Math.min(
    country.facts.length - 1,
    Math.floor(progress * country.facts.length),
  )

  return {
    country,
    lessonText,
    completedText,
    currentKey,
    upcomingText,
    progress,
    attempts,
    accuracy,
    altitude,
    glideSpeed,
    activeFactIndex,
    activeFact: country.facts[activeFactIndex] ?? country.facts[0],
    activeFinger: getFingerGuide(currentKey),
    isComplete: lessonText.length > 0 && state.cursor >= lessonText.length,
  }
}

export function getTypingFlightStyleMetrics(state: TypingSessionState) {
  const metrics = deriveTypingMetrics(state)

  return {
    flagColors: metrics.country.flagColors,
    gliderLeft: `${16 + metrics.progress * 64}%`,
    gliderTop: `${54 - metrics.progress * 19 + Math.min(state.mistakes, 5)}%`,
    gliderTilt:
      state.lastInput === 'wrong'
        ? '10deg'
        : `${Math.max(-10, 2 - state.streak * 0.35)}deg`,
    routeProgress: `${metrics.progress}`,
  }
}

import type { ActivityMode } from '../data'
import type { TypingDerivedMetrics, TypingInputMode } from '../lib/typing-engine'
import type { WorldDerivedMetrics } from '../lib/world-engine'

export type FlightAssistProfile = {
  inputResponsiveness: number
  coordinationAssist: number
  turbulenceDamping: number
  recoveryAssist: number
}

function clamp(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value))
}

type FlightAssistInput = {
  activityMode: ActivityMode
  typingMetrics: TypingDerivedMetrics
  worldMetrics: WorldDerivedMetrics
  lastInput: TypingInputMode
}

export function deriveFlightAssistProfile(
  input: FlightAssistInput,
): FlightAssistProfile {
  const discipline = clamp(
    input.typingMetrics.accuracy / 100 +
      input.typingMetrics.progress * 0.18 +
      Math.min(input.typingMetrics.country.lessonWords.length, 14) * 0.01,
    0.3,
    1.45,
  )
  const mistakePenalty = input.lastInput === 'wrong' ? 0.12 : 0
  const activeFlightBoost = input.activityMode.isActiveFlight ? 1 : 0.72

  return {
    inputResponsiveness: clamp(
      0.84 +
        discipline * 0.2 +
        input.activityMode.typingInfluence * 0.08 -
        mistakePenalty,
      0.72,
      1.08,
    ),
    coordinationAssist: clamp(
      discipline * 0.22 +
        input.activityMode.typingInfluence * 0.08 -
        input.worldMetrics.turbulence * 0.1 -
        mistakePenalty * 0.6,
      0,
      0.36,
    ),
    turbulenceDamping: clamp(
      discipline * 0.28 * activeFlightBoost -
        input.worldMetrics.turbulence * 0.06 -
        mistakePenalty,
      0,
      0.32,
    ),
    recoveryAssist: clamp(
      discipline * 0.2 * activeFlightBoost +
        (input.lastInput === 'correct' ? 0.03 : 0) -
        mistakePenalty * 0.4,
      0,
      0.24,
    ),
  }
}

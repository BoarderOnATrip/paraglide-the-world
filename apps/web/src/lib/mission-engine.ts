import type { ActivityMode } from '../data'
import type { CountryContent, CountryId } from './typing-content'

type MissionCheckpointKind = 'launch' | 'checkpoint' | 'landing'
type MissionCheckpointStatus = 'cleared' | 'active' | 'upcoming'
type MissionRating = 'gold' | 'silver' | 'bronze'

export type MissionSessionState = {
  activityModeId: ActivityMode['id']
  countryId: CountryId
  previousProgress: number
  runCount: number
  clearedCheckpointIds: string[]
  lastClearedCheckpointId: string | null
  lastResult: MissionResult | null
}

export type MissionResult = {
  rating: MissionRating
  title: string
  summary: string
  distanceKm: number
  mistakes: number
  accuracy: number
  runNumber: number
}

export type MissionCheckpointView = {
  id: string
  label: string
  kind: MissionCheckpointKind
  progress: number
  status: MissionCheckpointStatus
}

export type MissionView = {
  checkpoints: MissionCheckpointView[]
  activeCheckpoint: MissionCheckpointView | null
  previewRating: MissionRating
  previewLabel: string
  objectiveTitle: string
  objectiveSummary: string
  clearedCount: number
  totalCount: number
  lastResult: MissionResult | null
  runCount: number
}

export type MissionAdvanceInput = {
  activityMode: ActivityMode
  country: CountryContent
  progress: number
  distanceKm: number
  accuracy: number
  mistakes: number
}

type BaseMissionCheckpoint = {
  id: string
  label: string
  kind: MissionCheckpointKind
  progress: number
}

function getCheckpointId(countryId: CountryId, label: string) {
  return `${countryId}-${label.toLowerCase().replace(/\s+/g, '-')}`
}

function getMissionRating(
  accuracy: number,
  mistakes: number,
  isActiveFlight: boolean,
): MissionRating {
  if (isActiveFlight && accuracy >= 96 && mistakes <= 2) {
    return 'gold'
  }

  if (accuracy >= 90 && mistakes <= 5) {
    return 'silver'
  }

  return 'bronze'
}

function getRatingLabel(rating: MissionRating) {
  switch (rating) {
    case 'gold':
      return 'Gold line'
    case 'silver':
      return 'Stable line'
    case 'bronze':
      return 'Recovery line'
    default:
      return 'Live run'
  }
}

function createMissionResult(
  input: MissionAdvanceInput,
  runNumber: number,
): MissionResult {
  const rating = getMissionRating(
    input.accuracy,
    input.mistakes,
    input.activityMode.isActiveFlight,
  )

  if (!input.activityMode.isActiveFlight) {
    return {
      rating,
      title: 'Scenic loop complete',
      summary:
        'You finished the route in a social mode. Switch back to paragliding when you want the full skill run.',
      distanceKm: input.distanceKm,
      mistakes: input.mistakes,
      accuracy: input.accuracy,
      runNumber,
    }
  }

  if (rating === 'gold') {
    return {
      rating,
      title: 'Clean landing',
      summary:
        'Accuracy stayed high through the full route. This run reads like confident, efficient finger control.',
      distanceKm: input.distanceKm,
      mistakes: input.mistakes,
      accuracy: input.accuracy,
      runNumber,
    }
  }

  if (rating === 'silver') {
    return {
      rating,
      title: 'Stable finish',
      summary:
        'You held the wing together and completed the line cleanly. There is still room to tighten the hand work.',
      distanceKm: input.distanceKm,
      mistakes: input.mistakes,
      accuracy: input.accuracy,
      runNumber,
    }
  }

  return {
    rating,
    title: 'Recovery landing',
    summary:
      'You brought the route home under pressure. Clean up the finger timing for a more efficient next pass.',
    distanceKm: input.distanceKm,
    mistakes: input.mistakes,
    accuracy: input.accuracy,
    runNumber,
  }
}

function createBaseCheckpoints(country: CountryContent): BaseMissionCheckpoint[] {
  const lastIndex = Math.max(country.waypoints.length - 1, 1)

  return country.waypoints.map((waypoint, index) => ({
    id: getCheckpointId(country.id, waypoint),
    label: waypoint,
    kind:
      index === 0
        ? 'launch'
        : index === country.waypoints.length - 1
          ? 'landing'
          : 'checkpoint',
    progress: index === country.waypoints.length - 1 ? 1 : index / lastIndex,
  }))
}

function crossesProgress(
  previousProgress: number,
  currentProgress: number,
  checkpointProgress: number,
) {
  if (currentProgress < previousProgress) {
    return checkpointProgress > previousProgress || checkpointProgress <= currentProgress
  }

  return checkpointProgress > previousProgress && checkpointProgress <= currentProgress
}

export function createInitialMissionSession(
  activityModeId: ActivityMode['id'],
  countryId: CountryId,
): MissionSessionState {
  return {
    activityModeId,
    countryId,
    previousProgress: 0,
    runCount: 0,
    clearedCheckpointIds: [getCheckpointId(countryId, 'Launch')],
    lastClearedCheckpointId: getCheckpointId(countryId, 'Launch'),
    lastResult: null,
  }
}

export function advanceMissionSession(
  state: MissionSessionState,
  input: MissionAdvanceInput,
): MissionSessionState {
  const checkpoints = createBaseCheckpoints(input.country)
  const launchCheckpoint = checkpoints[0]
  const landingCheckpoint = checkpoints[checkpoints.length - 1]
  const nextCleared = new Set(
    state.clearedCheckpointIds.length > 0
      ? state.clearedCheckpointIds
      : launchCheckpoint == null
        ? []
        : [launchCheckpoint.id],
  )
  let lastClearedCheckpointId = state.lastClearedCheckpointId
  let checkpointCleared = false

  for (const checkpoint of checkpoints.slice(1, -1)) {
    if (
      !nextCleared.has(checkpoint.id) &&
      crossesProgress(state.previousProgress, input.progress, checkpoint.progress)
    ) {
      nextCleared.add(checkpoint.id)
      lastClearedCheckpointId = checkpoint.id
      checkpointCleared = true
    }
  }

  if (input.progress < state.previousProgress && landingCheckpoint != null) {
    const runNumber = state.runCount + 1

    return {
      activityModeId: input.activityMode.id,
      countryId: input.country.id,
      previousProgress: input.progress,
      runCount: runNumber,
      clearedCheckpointIds: launchCheckpoint == null ? [] : [launchCheckpoint.id],
      lastClearedCheckpointId: landingCheckpoint.id,
      lastResult: createMissionResult(input, runNumber),
    }
  }

  if (
    state.countryId === input.country.id &&
    state.activityModeId === input.activityMode.id &&
    state.previousProgress === input.progress &&
    !checkpointCleared
  ) {
    return state
  }

  return {
    ...state,
    activityModeId: input.activityMode.id,
    countryId: input.country.id,
    previousProgress: input.progress,
    clearedCheckpointIds: Array.from(nextCleared),
    lastClearedCheckpointId,
  }
}

export function deriveMissionView(
  state: MissionSessionState,
  country: CountryContent,
  activityMode: ActivityMode,
  accuracy: number,
  mistakes: number,
): MissionView {
  const checkpoints = createBaseCheckpoints(country)
  const clearedCheckpointIds = new Set(state.clearedCheckpointIds)
  let activeCheckpoint: MissionCheckpointView | null = null

  const checkpointViews = checkpoints.map((checkpoint, index) => {
    const isCleared =
      checkpoint.kind === 'landing'
        ? false
        : clearedCheckpointIds.has(checkpoint.id)
    const status: MissionCheckpointStatus =
      isCleared
        ? 'cleared'
        : activeCheckpoint == null && index > 0
          ? 'active'
          : 'upcoming'
    const checkpointView = {
      ...checkpoint,
      status,
    } satisfies MissionCheckpointView

    if (status === 'active') {
      activeCheckpoint = checkpointView
    }

    return checkpointView
  })

  if (activeCheckpoint == null) {
    activeCheckpoint = checkpointViews[checkpointViews.length - 1] ?? null
  }

  const previewRating = getMissionRating(
    accuracy,
    mistakes,
    activityMode.isActiveFlight,
  )
  const objectiveTitle =
    activeCheckpoint == null
      ? 'Start a new run'
      : activeCheckpoint.kind === 'launch'
        ? `Set up ${activeCheckpoint.label}`
        : activeCheckpoint.kind === 'landing'
          ? `Land at ${activeCheckpoint.label}`
          : `Clear ${activeCheckpoint.label}`
  const objectiveSummary =
    activeCheckpoint == null
      ? 'The route is ready whenever you are.'
      : activityMode.isActiveFlight
        ? `Hold disciplined hand movement and keep the wing clean until ${activeCheckpoint.label}.`
        : `Scenic travel is carrying the route forward. Switch to paragliding if you want the full skill challenge before ${activeCheckpoint.label}.`

  return {
    checkpoints: checkpointViews,
    activeCheckpoint,
    previewRating,
    previewLabel: getRatingLabel(previewRating),
    objectiveTitle,
    objectiveSummary,
    clearedCount: checkpointViews.filter(
      (checkpoint) =>
        checkpoint.kind !== 'launch' && checkpoint.status === 'cleared',
    ).length,
    totalCount: Math.max(checkpointViews.length - 1, 1),
    lastResult: state.lastResult,
    runCount: state.runCount,
  }
}

import {
  stepFlightState,
  type FlightSimState,
  type FlightStepInput,
  type LandingRating,
} from '../flight-model'

export type FlightScenarioFrameFactory = (
  frameIndex: number,
  currentState: FlightSimState,
) => FlightStepInput

export type FlightScenarioRunSummary = {
  requestedFrames: number
  completedFrames: number
  stoppedEarly: boolean
  landed: boolean
  crashed: boolean
  finalLandingRating: LandingRating
  finalLandingZoneDistanceMeters: number | null
  finalLandingApproachErrorDeg: number | null
  averageAirspeedKmh: number
  averageGroundSpeedKmh: number
  averageVerticalSpeedMetersPerSecond: number
  minGroundClearanceMeters: number
  maxGroundClearanceMeters: number
  peakRidgeLiftMetersPerSecond: number
  peakThermalLiftMetersPerSecond: number
  peakStallWarning: number
}

export type FlightScenarioRunResult = {
  states: FlightSimState[]
  summary: FlightScenarioRunSummary
}

function initializeSummaryState(initialState: FlightSimState) {
  return {
    airspeedSum: 0,
    groundSpeedSum: 0,
    verticalSpeedSum: 0,
    minGroundClearanceMeters: initialState.groundClearanceMeters,
    maxGroundClearanceMeters: initialState.groundClearanceMeters,
    peakRidgeLiftMetersPerSecond: initialState.ridgeLiftMetersPerSecond,
    peakThermalLiftMetersPerSecond: initialState.thermalLiftMetersPerSecond,
    peakStallWarning: initialState.stallWarning,
  }
}

export function runFlightScenario(
  initialState: FlightSimState,
  frameCount: number,
  frameFactory: FlightScenarioFrameFactory,
  options: {
    stopOnLanding?: boolean
  } = {},
): FlightScenarioRunResult {
  if (frameCount < 0) {
    throw new Error('frameCount must be non-negative')
  }

  const states: FlightSimState[] = [initialState]
  const summaryState = initializeSummaryState(initialState)
  const stopOnLanding = options.stopOnLanding ?? true
  let currentState = initialState
  let completedFrames = 0

  for (let frameIndex = 0; frameIndex < frameCount; frameIndex += 1) {
    if (stopOnLanding && currentState.flightPhase === 'landed') {
      break
    }

    if (stopOnLanding && currentState.flightPhase === 'crashed') {
      break
    }

    const frameInput = frameFactory(frameIndex, currentState)
    const nextState = stepFlightState(currentState, {
      ...frameInput,
      deltaSeconds: frameInput.deltaSeconds ?? 0.2,
    })

    states.push(nextState)
    currentState = nextState
    completedFrames += 1

    summaryState.airspeedSum += nextState.airspeedKmh
    summaryState.groundSpeedSum += nextState.groundSpeedKmh
    summaryState.verticalSpeedSum += nextState.verticalSpeedMetersPerSecond
    summaryState.minGroundClearanceMeters = Math.min(
      summaryState.minGroundClearanceMeters,
      nextState.groundClearanceMeters,
    )
    summaryState.maxGroundClearanceMeters = Math.max(
      summaryState.maxGroundClearanceMeters,
      nextState.groundClearanceMeters,
    )
    summaryState.peakRidgeLiftMetersPerSecond = Math.max(
      summaryState.peakRidgeLiftMetersPerSecond,
      nextState.ridgeLiftMetersPerSecond,
    )
    summaryState.peakThermalLiftMetersPerSecond = Math.max(
      summaryState.peakThermalLiftMetersPerSecond,
      nextState.thermalLiftMetersPerSecond,
    )
    summaryState.peakStallWarning = Math.max(
      summaryState.peakStallWarning,
      nextState.stallWarning,
    )
  }

  const divisor = Math.max(completedFrames, 1)

  return {
    states,
    summary: {
      requestedFrames: frameCount,
      completedFrames,
      stoppedEarly: completedFrames < frameCount,
      landed: currentState.flightPhase === 'landed',
      crashed: currentState.flightPhase === 'crashed',
      finalLandingRating: currentState.landingRating,
      finalLandingZoneDistanceMeters: currentState.landingZoneDistanceMeters,
      finalLandingApproachErrorDeg: currentState.landingApproachErrorDeg,
      averageAirspeedKmh: summaryState.airspeedSum / divisor,
      averageGroundSpeedKmh: summaryState.groundSpeedSum / divisor,
      averageVerticalSpeedMetersPerSecond:
        summaryState.verticalSpeedSum / divisor,
      minGroundClearanceMeters: summaryState.minGroundClearanceMeters,
      maxGroundClearanceMeters: summaryState.maxGroundClearanceMeters,
      peakRidgeLiftMetersPerSecond: summaryState.peakRidgeLiftMetersPerSecond,
      peakThermalLiftMetersPerSecond: summaryState.peakThermalLiftMetersPerSecond,
      peakStallWarning: summaryState.peakStallWarning,
    },
  }
}

export function runConstantFlightScenario(
  initialState: FlightSimState,
  frameCount: number,
  frameInput: FlightStepInput,
  options?: {
    stopOnLanding?: boolean
  },
) {
  return runFlightScenario(
    initialState,
    frameCount,
    () => frameInput,
    options,
  )
}

import {
  getAngleDeltaDegrees,
  getLocalOffsetMeters,
  projectOffsetMeters,
} from './geometry'
import { FLIGHT_TUNING } from './tuning'
import type { LandingRating } from './types'
import type { LandingZone } from '../sim/site-data'

export type LandingZoneMetrics = {
  distanceMeters: number
  approachErrorDeg: number
  isInside: boolean
}

const { landing } = FLIGHT_TUNING

export function getLandingZoneMetrics(
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

export function getLandingRating(
  verticalSpeedMetersPerSecond: number,
  groundSpeedKmh: number,
  bankDeg: number,
  landingMetrics: LandingZoneMetrics | null,
): LandingRating {
  const severityScore =
    Math.abs(verticalSpeedMetersPerSecond) *
      landing.verticalSpeedSeverityMultiplier +
    Math.max(0, groundSpeedKmh - landing.groundSpeedSeverityOffsetKmh) /
      landing.groundSpeedSeverityDivisor +
    Math.abs(bankDeg) / landing.bankSeverityDivisorDeg +
    (landingMetrics
      ? landingMetrics.approachErrorDeg / landing.approachErrorSeverityDivisorDeg
      : 0) +
    (landingMetrics
      ? landingMetrics.distanceMeters / landing.landingDistanceSeverityDivisorMeters
      : 0)

  if (severityScore < landing.smoothSeverityThreshold) {
    return 'smooth'
  }

  if (severityScore < landing.firmSeverityThreshold) {
    return 'firm'
  }

  if (severityScore < landing.hardSeverityThreshold) {
    return 'hard'
  }

  return 'crash'
}

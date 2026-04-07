export const METERS_PER_LATITUDE_DEGREE = 111_320

export type LocalOffsetMeters = {
  east: number
  north: number
}

export function clamp(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value))
}

export function wrapDegrees(value: number) {
  return (value % 360 + 360) % 360
}

export function approach(current: number, target: number, maxStep: number) {
  if (current < target) {
    return Math.min(current + maxStep, target)
  }

  return Math.max(current - maxStep, target)
}

export function toRadians(value: number) {
  return (value * Math.PI) / 180
}

export function toDegrees(value: number) {
  return (value * 180) / Math.PI
}

export function getAngleDeltaDegrees(a: number, b: number) {
  return ((a - b + 540) % 360) - 180
}

export function getMetersPerLongitudeDegree(latitude: number) {
  return Math.max(
    1,
    METERS_PER_LATITUDE_DEGREE * Math.cos((latitude * Math.PI) / 180),
  )
}

export function getLocalOffsetMeters(
  latitude: number,
  longitude: number,
  centerLatitude: number,
  centerLongitude: number,
): LocalOffsetMeters {
  return {
    north: (latitude - centerLatitude) * METERS_PER_LATITUDE_DEGREE,
    east:
      (longitude - centerLongitude) *
      getMetersPerLongitudeDegree((latitude + centerLatitude) / 2),
  }
}

export function getGroundDistanceMeters(
  latitudeA: number,
  longitudeA: number,
  latitudeB: number,
  longitudeB: number,
) {
  const offset = getLocalOffsetMeters(latitudeB, longitudeB, latitudeA, longitudeA)
  return Math.hypot(offset.north, offset.east)
}

export function projectOffsetMeters(offset: LocalOffsetMeters, headingDeg: number) {
  const headingRadians = toRadians(headingDeg)
  return (
    offset.east * Math.sin(headingRadians) +
    offset.north * Math.cos(headingRadians)
  )
}

export function offsetPositionByMeters(
  latitude: number,
  longitude: number,
  eastMeters: number,
  northMeters: number,
) {
  return {
    latitude: latitude + northMeters / METERS_PER_LATITUDE_DEGREE,
    longitude:
      longitude + eastMeters / getMetersPerLongitudeDegree(latitude),
  }
}

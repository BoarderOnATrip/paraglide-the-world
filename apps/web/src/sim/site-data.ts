export type ThermalSource = {
  id: string
  latitude: number
  longitude: number
  radiusMeters: number
  coreRadiusMeters?: number
  liftMetersPerSecond: number
  sinkRingMetersPerSecond?: number
  driftFactor?: number
}

export type RidgeBand = {
  latitude: number
  longitude: number
  axisHeadingDeg: number
  lengthMeters: number
  windwardDepthMeters: number
  leeDepthMeters: number
  peakLiftMetersPerSecond: number
  leeSinkMetersPerSecond: number
}

export type LandingZone = {
  latitude: number
  longitude: number
  headingDeg: number
  lengthMeters: number
  widthMeters: number
}

export type FlightSiteId = string

export type FlightSite = {
  id: FlightSiteId
  name: string
  countryId: string
  region: string
  country: string
  description: string
  latitude: number
  longitude: number
  launchAltitudeMeters: number
  spawnAglMeters: number
  prevailingWindHeadingDeg: number
  windSpeedKmh: number
  baseRidgeLiftMetersPerSecond: number
  routeLengthKm: number
  ridge?: RidgeBand
  landingZone?: LandingZone
  thermals: ThermalSource[]
}

export const FLIGHT_SITES: FlightSite[] = [
  {
    id: 'lauterbrunnen',
    name: 'Lauterbrunnen',
    countryId: 'switzerland',
    region: 'Bernese Alps',
    country: 'Switzerland',
    description:
      'A steep valley launch area that gives the first simulator slice dramatic relief, strong terrain readability, and obvious chase-camera payoff.',
    latitude: 46.5937,
    longitude: 7.9047,
    launchAltitudeMeters: 1485,
    spawnAglMeters: 140,
    prevailingWindHeadingDeg: 58,
    windSpeedKmh: 19,
    baseRidgeLiftMetersPerSecond: 1.4,
    routeLengthKm: 16,
    ridge: {
      latitude: 46.5928,
      longitude: 7.9036,
      axisHeadingDeg: 148,
      lengthMeters: 3300,
      windwardDepthMeters: 650,
      leeDepthMeters: 700,
      peakLiftMetersPerSecond: 2.4,
      leeSinkMetersPerSecond: 1.3,
    },
    landingZone: {
      latitude: 46.6022,
      longitude: 7.9106,
      headingDeg: 58,
      lengthMeters: 420,
      widthMeters: 120,
    },
    thermals: [
      {
        id: 'valley-mouth',
        latitude: 46.5986,
        longitude: 7.918,
        radiusMeters: 430,
        coreRadiusMeters: 150,
        liftMetersPerSecond: 2.2,
        sinkRingMetersPerSecond: 0.5,
        driftFactor: 0.45,
      },
      {
        id: 'waterfall-ribbon',
        latitude: 46.5879,
        longitude: 7.8986,
        radiusMeters: 360,
        coreRadiusMeters: 130,
        liftMetersPerSecond: 1.8,
        sinkRingMetersPerSecond: 0.35,
        driftFactor: 0.38,
      },
    ],
  },
  {
    id: 'rome',
    name: 'Rome',
    countryId: 'italy',
    region: 'Monte Mario',
    country: 'Italy',
    description:
      'A city glide seeded around Monte Mario so you can skim toward the Vatican, the Tiber, and central Rome without losing the home-row training loop.',
    latitude: 41.9281,
    longitude: 12.4472,
    launchAltitudeMeters: 138,
    spawnAglMeters: 150,
    prevailingWindHeadingDeg: 248,
    windSpeedKmh: 15,
    baseRidgeLiftMetersPerSecond: 0.6,
    routeLengthKm: 12,
    ridge: {
      latitude: 41.9272,
      longitude: 12.4468,
      axisHeadingDeg: 338,
      lengthMeters: 1900,
      windwardDepthMeters: 360,
      leeDepthMeters: 450,
      peakLiftMetersPerSecond: 1.1,
      leeSinkMetersPerSecond: 0.8,
    },
    landingZone: {
      latitude: 41.9124,
      longitude: 12.455,
      headingDeg: 238,
      lengthMeters: 360,
      widthMeters: 100,
    },
    thermals: [
      {
        id: 'vatican-core',
        latitude: 41.9036,
        longitude: 12.4548,
        radiusMeters: 520,
        coreRadiusMeters: 180,
        liftMetersPerSecond: 1.5,
        sinkRingMetersPerSecond: 0.25,
        driftFactor: 0.32,
      },
      {
        id: 'tiber-bend',
        latitude: 41.9148,
        longitude: 12.4651,
        radiusMeters: 460,
        coreRadiusMeters: 150,
        liftMetersPerSecond: 1.25,
        sinkRingMetersPerSecond: 0.22,
        driftFactor: 0.28,
      },
    ],
  },
  {
    id: 'istanbul',
    name: 'Istanbul',
    countryId: 'turkey',
    region: 'Camlica Hill',
    country: 'Turkey',
    description:
      'A Bosphorus launch line built for old-Constantinople sightseeing, with enough altitude to chase the skyline between Europe and Asia.',
    latitude: 41.0279,
    longitude: 29.0685,
    launchAltitudeMeters: 262,
    spawnAglMeters: 170,
    prevailingWindHeadingDeg: 32,
    windSpeedKmh: 18,
    baseRidgeLiftMetersPerSecond: 0.8,
    routeLengthKm: 14,
    ridge: {
      latitude: 41.0283,
      longitude: 29.0662,
      axisHeadingDeg: 318,
      lengthMeters: 2200,
      windwardDepthMeters: 420,
      leeDepthMeters: 520,
      peakLiftMetersPerSecond: 1.5,
      leeSinkMetersPerSecond: 0.95,
    },
    landingZone: {
      latitude: 41.0179,
      longitude: 29.0415,
      headingDeg: 22,
      lengthMeters: 380,
      widthMeters: 120,
    },
    thermals: [
      {
        id: 'uskudar-rise',
        latitude: 41.027,
        longitude: 29.0204,
        radiusMeters: 560,
        coreRadiusMeters: 180,
        liftMetersPerSecond: 1.7,
        sinkRingMetersPerSecond: 0.25,
        driftFactor: 0.34,
      },
      {
        id: 'bosphorus-mouth',
        latitude: 41.0417,
        longitude: 29.0409,
        radiusMeters: 610,
        coreRadiusMeters: 220,
        liftMetersPerSecond: 1.35,
        sinkRingMetersPerSecond: 0.3,
        driftFactor: 0.4,
      },
    ],
  },
]

export function getFlightSite(siteId: FlightSiteId) {
  return FLIGHT_SITES.find((site) => site.id === siteId) ?? FLIGHT_SITES[0]
}

import { useCallback, useEffect, useEffectEvent, useRef, useState } from 'react'
import { ACTIVITY_MODES, type ActivityModeId } from '../data'
import {
  advanceMissionSession,
  createInitialMissionSession,
  deriveMissionView,
  type MissionSessionState,
} from '../lib/mission-engine'
import {
  createInitialTypingSession,
  deriveTypingMetrics,
  processTypingKey,
  type TypingKeyResult,
  type TypingSessionState,
} from '../lib/typing-engine'
import {
  COUNTRIES,
  getCountryContent,
  type CountryId,
} from '../lib/typing-content'
import {
  createInitialWorldSession,
  deriveWorldMetrics,
  tickWorldSession,
  type WorldSessionState,
} from '../lib/world-engine'
import { deriveFlightAssistProfile } from './flight-assist'
import {
  createFlightScenarioSession,
  getFlightScenario,
  type FlightScenarioId,
} from '../flight/scenarios'
import {
  createInitialFlightState,
  resetFlightStateForSite,
  stepFlightState,
  type FlightSimState,
} from './flight-model'
import {
  applyHomeRowControlKey,
  DEFAULT_HOME_ROW_CONTROLS,
  normalizeHomeRowControlKey,
  stepHomeRowControls,
  type HomeRowControlState,
} from './home-row-controls'
import { getFlightSite, type FlightSiteId } from './site-data'
import type { AmbientAirState } from '../flight/types'

type SimulationSessionState = {
  siteId: FlightSiteId
  scenarioId: FlightScenarioId | null
  scenarioAtmosphere: AmbientAirState | null
  activityModeId: ActivityModeId
  terrainHeightMeters: number | null
  controls: HomeRowControlState
  flightState: FlightSimState
  typingSession: TypingSessionState
  worldSession: WorldSessionState
  missionSession: MissionSessionState
  lastTypingResult: TypingKeyResult | null
}

function isTextEntryTarget(target: EventTarget | null) {
  if (!(target instanceof HTMLElement)) {
    return false
  }

  const tagName = target.tagName

  return (
    target.isContentEditable ||
    tagName === 'INPUT' ||
    tagName === 'TEXTAREA' ||
    tagName === 'SELECT'
  )
}

function isSimulationControlKey(key: string) {
  const normalizedKey = normalizeHomeRowControlKey(key)

  return (
    normalizedKey === 'a' ||
    normalizedKey === 'f' ||
    normalizedKey === 'j' ||
    normalizedKey === ';' ||
    normalizedKey === 'space'
  )
}

function getActivityMode(activityModeId: ActivityModeId) {
  return ACTIVITY_MODES.find((mode) => mode.id === activityModeId) ?? ACTIVITY_MODES[0]
}

function getTypingCountryId(countryId: string): CountryId {
  const matchedCountry = COUNTRIES.find((country) => country.id === countryId)
  return matchedCountry?.id ?? COUNTRIES[0].id
}

type CreateSimulationSessionOptions = {
  siteId: FlightSiteId
  terrainHeightMeters?: number | null
  scenarioId?: FlightScenarioId | null
}

function createSimulationSessionState(
  options: CreateSimulationSessionOptions,
): SimulationSessionState {
  const scenarioId = options.scenarioId ?? null
  const scenarioSession =
    scenarioId == null
      ? null
      : createFlightScenarioSession(scenarioId, options.terrainHeightMeters)
  const site = scenarioSession?.site ?? getFlightSite(options.siteId)
  const countryId = getTypingCountryId(site.countryId)
  const activityModeId: ActivityModeId = 'paragliding'
  const initialTerrainHeightMeters =
    scenarioSession?.terrainHeightMeters ??
    options.terrainHeightMeters ??
    site.launchAltitudeMeters
  const scenarioAtmosphere = scenarioSession?.atmosphere ?? null

  return {
    siteId: site.id,
    scenarioId,
    scenarioAtmosphere,
    activityModeId,
    terrainHeightMeters: initialTerrainHeightMeters,
    controls: DEFAULT_HOME_ROW_CONTROLS,
    flightState:
      scenarioSession?.flightState ??
      createInitialFlightState(site, initialTerrainHeightMeters),
    typingSession: createInitialTypingSession(countryId),
    worldSession: {
      ...createInitialWorldSession(activityModeId, countryId),
      ...(scenarioAtmosphere
        ? {
            windHeading: scenarioAtmosphere.windHeadingDeg,
            windSpeedKmh: scenarioAtmosphere.windSpeedKmh,
            turbulence: scenarioAtmosphere.turbulence,
          }
        : {}),
    },
    missionSession: createInitialMissionSession(activityModeId, countryId),
    lastTypingResult: null,
  }
}

function getRouteProgress(distanceKm: number, routeLengthKm: number) {
  if (routeLengthKm <= 0) {
    return 0
  }

  return (distanceKm / routeLengthKm) % 1
}

export function useSimulationSession(initialSiteId: FlightSiteId) {
  const [session, setSession] = useState<SimulationSessionState>(() =>
    createSimulationSessionState({ siteId: initialSiteId }),
  )
  const terrainBootstrapRef = useRef(false)
  const selectedSite = getFlightSite(session.siteId)
  const selectedScenario = getFlightScenario(session.scenarioId)
  const selectedCountry = getCountryContent(getTypingCountryId(selectedSite.countryId))
  const activityMode = getActivityMode(session.activityModeId)
  const typingMetrics = deriveTypingMetrics(session.typingSession)
  const worldMetrics = deriveWorldMetrics(session.worldSession, activityMode)
  const flightAssist = deriveFlightAssistProfile({
    activityMode,
    typingMetrics,
    worldMetrics,
    lastInput: session.typingSession.lastInput,
  })
  const missionView = deriveMissionView(
    session.missionSession,
    selectedCountry,
    activityMode,
    typingMetrics.accuracy,
    session.typingSession.mistakes,
  )

  const selectSite = useCallback((siteId: FlightSiteId) => {
    terrainBootstrapRef.current = false
    setSession(
      createSimulationSessionState({
        siteId,
        scenarioId: null,
      }),
    )
  }, [])

  const selectScenario = useCallback((scenarioId: FlightScenarioId | null) => {
    terrainBootstrapRef.current = false
    setSession((currentSession) => {
      const scenario = getFlightScenario(scenarioId)

      return createSimulationSessionState({
        siteId: scenario?.siteId ?? currentSession.siteId,
        terrainHeightMeters:
          scenario == null || scenario.siteId === currentSession.siteId
            ? currentSession.terrainHeightMeters
            : undefined,
        scenarioId,
      })
    })
  }, [])

  const resetRun = useCallback(() => {
    terrainBootstrapRef.current = true
    setSession((currentSession) =>
      createSimulationSessionState({
        siteId: currentSession.siteId,
        terrainHeightMeters: currentSession.terrainHeightMeters,
        scenarioId: currentSession.scenarioId,
      }),
    )
  }, [])

  const handleTerrainSample = useCallback((nextTerrainHeightMeters: number | null) => {
    if (nextTerrainHeightMeters == null) {
      return
    }

    setSession((currentSession) => {
      if (
        terrainBootstrapRef.current &&
        currentSession.terrainHeightMeters === nextTerrainHeightMeters
      ) {
        return currentSession
      }

      if (!terrainBootstrapRef.current) {
        terrainBootstrapRef.current = true

        if (currentSession.scenarioId) {
          const scenarioSession = createFlightScenarioSession(
            currentSession.scenarioId,
            nextTerrainHeightMeters,
          )

          return {
            ...currentSession,
            siteId: scenarioSession.site.id,
            terrainHeightMeters: nextTerrainHeightMeters,
            scenarioAtmosphere: scenarioSession.atmosphere,
            flightState: scenarioSession.flightState,
            worldSession: {
              ...currentSession.worldSession,
              windHeading: scenarioSession.atmosphere.windHeadingDeg,
              windSpeedKmh: scenarioSession.atmosphere.windSpeedKmh,
              turbulence: scenarioSession.atmosphere.turbulence,
            },
          }
        }

        return {
          ...currentSession,
          terrainHeightMeters: nextTerrainHeightMeters,
          flightState: resetFlightStateForSite(
            getFlightSite(currentSession.siteId),
            nextTerrainHeightMeters,
          ),
        }
      }

      return {
        ...currentSession,
        terrainHeightMeters: nextTerrainHeightMeters,
      }
    })
  }, [])

  const handleKeyChange = useEffectEvent((key: string, isPressed: boolean) => {
    let handled = false

    setSession((currentSession) => {
      let nextSession = currentSession

      if (isSimulationControlKey(key)) {
        const nextControls = applyHomeRowControlKey(currentSession.controls, key, isPressed)

        if (nextControls !== currentSession.controls) {
          nextSession = {
            ...nextSession,
            controls: nextControls,
          }
        }

        handled = true
      }

      if (!isPressed) {
        return nextSession
      }

      const typingResult = processTypingKey(nextSession.typingSession, key)

      if (typingResult.ignored) {
        return nextSession
      }

      handled = true

      return {
        ...nextSession,
        typingSession: typingResult.state,
        lastTypingResult: typingResult,
      }
    })

    return handled
  })

  const tickSession = useEffectEvent((deltaMs: number) => {
    setSession((currentSession) => {
      const site = getFlightSite(currentSession.siteId)
      const country = getCountryContent(getTypingCountryId(site.countryId))
      const nextActivityMode = getActivityMode(currentSession.activityModeId)
      const nextTypingMetrics = deriveTypingMetrics(currentSession.typingSession)
      const tickedWorldSession = tickWorldSession(currentSession.worldSession, {
        deltaMs,
        activityMode: nextActivityMode,
        countryId: country.id,
        accuracy: nextTypingMetrics.accuracy,
        streak: currentSession.typingSession.streak,
        mistakes: currentSession.typingSession.mistakes,
      })
      const effectiveAtmosphere = currentSession.scenarioAtmosphere
        ? {
            windHeadingDeg: currentSession.scenarioAtmosphere.windHeadingDeg,
            windSpeedKmh: currentSession.scenarioAtmosphere.windSpeedKmh,
            turbulence: currentSession.scenarioAtmosphere.turbulence,
          }
        : {
            windHeadingDeg: tickedWorldSession.windHeading,
            windSpeedKmh: tickedWorldSession.windSpeedKmh,
            turbulence: tickedWorldSession.turbulence,
          }
      const atmosphereAlignedWorldSession = currentSession.scenarioAtmosphere
        ? {
            ...tickedWorldSession,
            windHeading: currentSession.scenarioAtmosphere.windHeadingDeg,
            windSpeedKmh: currentSession.scenarioAtmosphere.windSpeedKmh,
            turbulence: currentSession.scenarioAtmosphere.turbulence,
          }
        : tickedWorldSession
      const nextControls = stepHomeRowControls(
        currentSession.controls,
        deltaMs / 1000,
      )
      const nextWorldMetrics = deriveWorldMetrics(
        atmosphereAlignedWorldSession,
        nextActivityMode,
      )
      const nextFlightAssist = deriveFlightAssistProfile({
        activityMode: nextActivityMode,
        typingMetrics: nextTypingMetrics,
        worldMetrics: nextWorldMetrics,
        lastInput: currentSession.typingSession.lastInput,
      })
      const nextFlightState = stepFlightState(currentSession.flightState, {
        deltaSeconds: deltaMs / 1000,
        controls: nextControls,
        site,
        terrainHeightMeters: currentSession.terrainHeightMeters,
        atmosphere: effectiveAtmosphere,
        assist: nextFlightAssist,
      })
      const routeProgress = getRouteProgress(nextFlightState.distanceKm, site.routeLengthKm)
      const nextWorldSession = {
        ...atmosphereAlignedWorldSession,
        distanceKm: nextFlightState.distanceKm,
        travelProgress: routeProgress,
      }
      const nextMissionSession = advanceMissionSession(currentSession.missionSession, {
        activityMode: nextActivityMode,
        country,
        progress: routeProgress,
        distanceKm: Number(nextFlightState.distanceKm.toFixed(1)),
        accuracy: nextTypingMetrics.accuracy,
        mistakes: currentSession.typingSession.mistakes,
      })

      return {
        ...currentSession,
        controls: nextControls,
        worldSession: nextWorldSession,
        flightState: nextFlightState,
        missionSession: nextMissionSession,
      }
    })
  })

  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if (isTextEntryTarget(event.target)) {
        return
      }

      if (event.key.toLowerCase() === 'r') {
        event.preventDefault()
        resetRun()
        return
      }

      if (handleKeyChange(event.key, true)) {
        event.preventDefault()
      }
    }

    const handleKeyUp = (event: KeyboardEvent) => {
      if (handleKeyChange(event.key, false)) {
        event.preventDefault()
      }
    }

    window.addEventListener('keydown', handleKeyDown)
    window.addEventListener('keyup', handleKeyUp)

    return () => {
      window.removeEventListener('keydown', handleKeyDown)
      window.removeEventListener('keyup', handleKeyUp)
    }
  }, [resetRun])

  useEffect(() => {
    let lastFrame = performance.now()

    const intervalId = window.setInterval(() => {
      const now = performance.now()
      tickSession(now - lastFrame)
      lastFrame = now
    }, 50)

    return () => {
      window.clearInterval(intervalId)
    }
  }, [])

  return {
    activityMode,
    controls: session.controls,
    flightAssist,
    flightState: session.flightState,
    missionView,
    resetRun,
    selectScenario,
    selectSite,
    selectedCountry,
    selectedScenario,
    selectedSite,
    terrainHeightMeters: session.terrainHeightMeters,
    typingMetrics,
    typingSession: session.typingSession,
    worldMetrics,
    worldSession: session.worldSession,
    lastTypingResult: session.lastTypingResult,
    handleTerrainSample,
  }
}

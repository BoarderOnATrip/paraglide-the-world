import { useCallback, useEffect, useState } from 'react'
import { GooglePhotorealisticWorld } from './GooglePhotorealisticWorld'
import { HOME_ROW_CONTROL_GUIDE } from '../../sim/home-row-controls'
import { FLIGHT_SITES } from '../../sim/site-data'
import { useSimulationSession } from '../../sim/useSimulationSession'
import {
  DEFAULT_GOOGLE_WORLD_MODE,
  GOOGLE_WORLD_MODE_OPTIONS,
  type GoogleWorldMode,
} from '../../sim/world-mode'
import '../../simulator.css'

function getWorldSetupCopy(worldMode: GoogleWorldMode) {
  return worldMode === 'premium-3d'
    ? 'Set VITE_GOOGLE_MAPS_API_KEY to stream Google photorealistic 3D terrain.'
    : 'Set VITE_GOOGLE_MAPS_API_KEY to stream Google satellite tiles.'
}

function formatTypingKey(key: string) {
  return key === ' ' ? 'SPACE' : key.toUpperCase()
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

export function SimulatorApp() {
  const [hudMode, setHudMode] = useState<'panels' | 'flight'>('panels')
  const [worldMode, setWorldMode] = useState<GoogleWorldMode>(
    DEFAULT_GOOGLE_WORLD_MODE,
  )
  const [worldStatus, setWorldStatus] = useState<
    'config-needed' | 'loading' | 'ready' | 'error'
  >('config-needed')
  const [worldDetail, setWorldDetail] = useState(
    getWorldSetupCopy(DEFAULT_GOOGLE_WORLD_MODE),
  )
  const {
    activityMode,
    controls,
    flightAssist,
    flightState,
    missionView,
    resetRun,
    selectSite,
    selectedCountry,
    selectedSite,
    terrainHeightMeters,
    typingMetrics,
    typingSession,
    worldMetrics,
    lastTypingResult,
    handleTerrainSample,
  } = useSimulationSession(FLIGHT_SITES[0].id)
  const selectedWorldMode =
    GOOGLE_WORLD_MODE_OPTIONS.find((option) => option.id === worldMode) ??
    GOOGLE_WORLD_MODE_OPTIONS[0]
  const googleMapsApiKey = import.meta.env.VITE_GOOGLE_MAPS_API_KEY?.trim() || null

  const activeFactIndex =
    selectedCountry.facts.length === 0
      ? -1
      : Math.min(
          selectedCountry.facts.length - 1,
          Math.floor(worldMetrics.travelProgress * selectedCountry.facts.length),
        )
  const activeFact =
    activeFactIndex >= 0
      ? selectedCountry.facts[activeFactIndex] ?? selectedCountry.facts[0] ?? ''
      : ''
  const handleWorldStatusChange = useCallback(
    (status: 'config-needed' | 'loading' | 'ready' | 'error', detail?: string) => {
      setWorldStatus(status)
      setWorldDetail(detail ?? '')
    },
    [],
  )
  const toggleHudMode = () => {
    setHudMode((currentMode) => (currentMode === 'panels' ? 'flight' : 'panels'))
  }

  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if (event.key !== 'Tab' || isTextEntryTarget(event.target)) {
        return
      }

      event.preventDefault()
      toggleHudMode()
    }

    window.addEventListener('keydown', handleKeyDown)

    return () => {
      window.removeEventListener('keydown', handleKeyDown)
    }
  }, [])

  return (
    <div className={`sim-shell sim-shell--${hudMode}`}>
      <GooglePhotorealisticWorld
        key={`${selectedSite.id}-${worldMode}`}
        apiKey={googleMapsApiKey}
        mode={worldMode}
        site={selectedSite}
        flightState={flightState}
        onTerrainSample={handleTerrainSample}
        onWorldStatusChange={handleWorldStatusChange}
      />

      <div className="sim-shell__scrim" />

      <div className="sim-topbar">
        <button className="sim-view-toggle" onClick={toggleHudMode} type="button">
          {hudMode === 'panels' ? 'Flight View' : 'Show Panels'}
          <span>Tab</span>
        </button>
      </div>

      {hudMode === 'panels' ? (
        <>
          <header className="sim-hero">
            <div>
              <p className="sim-kicker">photorealistic typing flight simulator</p>
              <h1>Paraglide the World</h1>
              <p className="sim-copy">
                The wing is now driven by one live session. Home-row controls fly the
                glider with progressive brake travel, while typing discipline shapes
                control smoothness, stability, route progression, and mission quality.
              </p>
            </div>
            <div className={`sim-status sim-status--${worldStatus}`}>
              <strong>
                {worldStatus === 'ready'
                  ? `${selectedWorldMode.label} live`
                  : `${selectedWorldMode.label} setup`}
              </strong>
              <span>{worldDetail}</span>
            </div>
          </header>

          <main className="sim-grid">
            <section className="sim-panel sim-panel--wide">
              <div className="sim-panel__header">
                <p className="sim-panel__eyebrow">Active Route</p>
                <h2>
                  {selectedSite.name}, {selectedSite.country}
                </h2>
              </div>
              <p className="sim-panel__body">{selectedCountry.tagline}</p>
              <p className="sim-panel__body sim-panel__body--tight">
                {selectedCountry.mission}
              </p>
              <div className="sim-choice-group">
                <p className="sim-choice-group__label">Launch Site</p>
                <div className="sim-choice-grid">
                  {FLIGHT_SITES.map((site) => {
                    const isSelected = site.id === selectedSite.id

                    return (
                      <button
                        key={site.id}
                        className={`sim-choice${isSelected ? ' is-selected' : ''}`}
                        onClick={() => selectSite(site.id)}
                        type="button"
                      >
                        <strong>{site.name}</strong>
                        <span>
                          {site.region}, {site.country}
                        </span>
                      </button>
                    )
                  })}
                </div>
              </div>
              <div className="sim-choice-group">
                <p className="sim-choice-group__label">World Tier</p>
                <div className="sim-choice-grid sim-choice-grid--tiers">
                  {GOOGLE_WORLD_MODE_OPTIONS.map((option) => {
                    const isSelected = option.id === worldMode

                    return (
                      <button
                        key={option.id}
                        className={`sim-choice${isSelected ? ' is-selected' : ''}`}
                        onClick={() => setWorldMode(option.id)}
                        type="button"
                      >
                        <strong>{option.label}</strong>
                        <span>{option.summary}</span>
                      </button>
                    )
                  })}
                </div>
              </div>
              <div className="sim-site-meta">
                <span>{selectedCountry.route}</span>
                <span>{activityMode.name}</span>
                <span>{Math.round(flightState.distanceKm * 10) / 10} km flown</span>
                <span>{Math.round(flightState.elapsedSeconds)} s elapsed</span>
              </div>
            </section>

            <section className="sim-panel">
              <div className="sim-panel__header">
                <p className="sim-panel__eyebrow">Flight State</p>
                <h2>Wing telemetry</h2>
              </div>
              <div className="sim-stat-grid">
                <article className="sim-stat">
                  <strong>{Math.round(flightState.airspeedKmh)}</strong>
                  <span>airspeed km/h</span>
                </article>
                <article className="sim-stat">
                  <strong>{Math.round(flightState.groundSpeedKmh)}</strong>
                  <span>groundspeed km/h</span>
                </article>
                <article className="sim-stat">
                  <strong>{flightState.verticalSpeedMetersPerSecond.toFixed(1)}</strong>
                  <span>vario m/s</span>
                </article>
                <article className="sim-stat">
                  <strong>{Math.round(flightState.groundClearanceMeters)}</strong>
                  <span>ground clearance m</span>
                </article>
                <article className="sim-stat">
                  <strong>{Math.round(flightState.bankDeg)}°</strong>
                  <span>bank angle</span>
                </article>
                <article className="sim-stat">
                  <strong>{Math.round(flightState.headingDeg)}°</strong>
                  <span>heading</span>
                </article>
                <article className="sim-stat">
                  <strong>{Math.round(flightState.turnRateDegPerSecond ?? 0)}°/s</strong>
                  <span>turn rate</span>
                </article>
                <article className="sim-stat">
                  <strong>{flightState.flightPhase}</strong>
                  <span>flight phase</span>
                </article>
                <article className="sim-stat">
                  <strong>{flightState.landingRating}</strong>
                  <span>last landing</span>
                </article>
              </div>
            </section>

            <section className="sim-panel">
              <div className="sim-panel__header">
                <p className="sim-panel__eyebrow">Control Discipline</p>
                <h2>
                  {typingMetrics.activeFinger.label} on{' '}
                  {formatTypingKey(typingMetrics.currentKey)}
                </h2>
              </div>
              <div className="sim-stat-grid">
                <article className="sim-stat">
                  <strong>{typingMetrics.accuracy}%</strong>
                  <span>accuracy</span>
                </article>
                <article className="sim-stat">
                  <strong>{typingSession.streak}</strong>
                  <span>live streak</span>
                </article>
                <article className="sim-stat">
                  <strong>{Math.round(typingMetrics.progress * 100)}%</strong>
                  <span>lesson progress</span>
                </article>
              </div>
              <div className="sim-lesson">
                <span className="sim-lesson__done">{typingMetrics.completedText}</span>
                <span className="sim-lesson__current">
                  {formatTypingKey(typingMetrics.currentKey)}
                </span>
                <span className="sim-lesson__next">{typingMetrics.upcomingText}</span>
              </div>
              <p className="sim-panel__body sim-panel__body--tight">
                {typingMetrics.activeFinger.reminder}
              </p>
              <div className="sim-pill-row">
                <span className={`sim-pill sim-pill--${typingSession.lastInput}`}>
                  {typingSession.lastInput === 'idle'
                    ? 'Awaiting input'
                    : typingSession.lastInput === 'correct'
                      ? 'Clean input'
                      : 'Recovery input'}
                </span>
                <span className="sim-pill">
                  {lastTypingResult?.completedLesson
                    ? 'Lesson loop cleared'
                    : `${typingSession.mistakes} mistakes`}
                </span>
              </div>
            </section>

            <section className="sim-panel">
              <div className="sim-panel__header">
                <p className="sim-panel__eyebrow">Mission</p>
                <h2>{missionView.objectiveTitle}</h2>
              </div>
              <p className="sim-panel__body sim-panel__body--tight">
                {missionView.objectiveSummary}
              </p>
              <div className="sim-pill-row">
                <span className={`sim-pill sim-pill--${missionView.previewRating}`}>
                  {missionView.previewLabel}
                </span>
                <span className="sim-pill">
                  {missionView.clearedCount}/{missionView.totalCount} cleared
                </span>
              </div>
              <div className="sim-checkpoint-list">
                {missionView.checkpoints.map((checkpoint) => (
                  <article
                    key={checkpoint.id}
                    className={`sim-checkpoint sim-checkpoint--${checkpoint.status}`}
                  >
                    <strong>{checkpoint.label}</strong>
                    <span>
                      {checkpoint.kind === 'launch'
                        ? 'Launch'
                        : checkpoint.kind === 'landing'
                          ? 'Landing'
                          : 'Checkpoint'}
                    </span>
                  </article>
                ))}
              </div>
              {missionView.lastResult ? (
                <div
                  className={`sim-result sim-result--${missionView.lastResult.rating}`}
                >
                  <strong>{missionView.lastResult.title}</strong>
                  <span>
                    {missionView.lastResult.accuracy}% accuracy ·{' '}
                    {missionView.lastResult.mistakes} mistakes
                  </span>
                </div>
              ) : null}
            </section>

            <section className="sim-panel">
              <div className="sim-panel__header">
                <p className="sim-panel__eyebrow">Flight Trainer</p>
                <h2>Discipline shapes handling, not lift</h2>
              </div>
              <div className="sim-lift-stack">
                <div>
                  <strong>{Math.round(flightAssist.inputResponsiveness * 100)}%</strong>
                  <span>input responsiveness</span>
                </div>
                <div>
                  <strong>{worldMetrics.turbulencePercent}%</strong>
                  <span>ambient turbulence</span>
                </div>
                <div>
                  <strong>{Math.round(flightAssist.coordinationAssist * 100)}%</strong>
                  <span>coordination assist</span>
                </div>
                <div>
                  <strong>{Math.round(worldMetrics.windSpeedKmh)} km/h</strong>
                  <span>{worldMetrics.windLabel}</span>
                </div>
                <div>
                  <strong>{Math.round(flightAssist.turbulenceDamping * 100)}%</strong>
                  <span>gust damping</span>
                </div>
                <div>
                  <strong>{Math.round(flightAssist.recoveryAssist * 100)}%</strong>
                  <span>recovery assist</span>
                </div>
                <div>
                  <strong>{Math.round((flightState.stallWarning ?? 0) * 100)}%</strong>
                  <span>stall warning</span>
                </div>
                <div>
                  <strong>{flightState.ridgeLiftMetersPerSecond.toFixed(1)} m/s</strong>
                  <span>ridge lift</span>
                </div>
                <div>
                  <strong>{flightState.thermalLiftMetersPerSecond.toFixed(1)} m/s</strong>
                  <span>thermal lift</span>
                </div>
                <div>
                  <strong>{flightState.airMassSinkMetersPerSecond.toFixed(1)} m/s</strong>
                  <span>lee / edge sink</span>
                </div>
                <div>
                  <strong>{Math.round(flightState.flareEffectiveness * 100)}%</strong>
                  <span>flare timing</span>
                </div>
                <div>
                  <strong>
                    {Math.round(
                      terrainHeightMeters ?? selectedSite.launchAltitudeMeters,
                    )}{' '}
                    m
                  </strong>
                  <span>
                    {worldMode === 'premium-3d'
                      ? 'sampled terrain floor'
                      : 'shared launch-floor baseline'}
                  </span>
                </div>
              </div>
            </section>

            <section className="sim-panel">
              <div className="sim-panel__header">
                <p className="sim-panel__eyebrow">Route Radio</p>
                <h2>{selectedCountry.radioStation}</h2>
              </div>
              <p className="sim-panel__body sim-panel__body--tight">{activeFact}</p>
              <div className="sim-controls">
                {HOME_ROW_CONTROL_GUIDE.map((control) => {
                  const isActive =
                    (control.keyLabel === 'A' && controls.weightLeft) ||
                    (control.keyLabel === 'F' && controls.leftBrake) ||
                    (control.keyLabel === 'J' && controls.rightBrake) ||
                    (control.keyLabel === ';' && controls.weightRight) ||
                    (control.keyLabel === 'Space' && controls.speedBar)

                  return (
                    <article
                      key={control.keyLabel}
                      className={`sim-control${isActive ? ' is-active' : ''}`}
                    >
                      <strong>{control.keyLabel}</strong>
                      <span>{control.description}</span>
                    </article>
                  )
                })}
              </div>
              <div className="sim-actions">
                <button className="sim-button" onClick={resetRun} type="button">
                  Reset Run
                </button>
                <span>Type the lesson, work the air, and tap `R` to relaunch.</span>
              </div>
            </section>
          </main>
        </>
      ) : (
        <section className="sim-flight-hud">
          <div className="sim-flight-hud__top">
            <div className="sim-flight-card">
              <p className="sim-kicker">Flight View</p>
              <h2>
                {selectedSite.name}, {selectedSite.country}
              </h2>
              <p>{selectedCountry.route}</p>
            </div>
            <div className={`sim-status sim-status--${worldStatus} sim-status--compact`}>
              <strong>
                {worldStatus === 'ready'
                  ? `${selectedWorldMode.label} live`
                  : `${selectedWorldMode.label} setup`}
              </strong>
              <span>{worldDetail}</span>
            </div>
          </div>

          <div className="sim-flight-hud__strip">
            <article className="sim-flight-chip">
              <strong>{Math.round(flightState.airspeedKmh)}</strong>
              <span>airspeed</span>
            </article>
            <article className="sim-flight-chip">
              <strong>{Math.round(flightState.groundClearanceMeters)} m</strong>
              <span>clearance</span>
            </article>
            <article className="sim-flight-chip">
              <strong>{flightState.verticalSpeedMetersPerSecond.toFixed(1)} m/s</strong>
              <span>vario</span>
            </article>
            <article className="sim-flight-chip">
              <strong>{typingMetrics.accuracy}%</strong>
              <span>accuracy</span>
            </article>
            <article className="sim-flight-chip">
              <strong>{flightState.flightPhase}</strong>
              <span>phase</span>
            </article>
            <article className="sim-flight-chip">
              <strong>{Math.round((flightState.stallWarning ?? 0) * 100)}%</strong>
              <span>stall</span>
            </article>
            <article className="sim-flight-chip">
              <strong>{formatTypingKey(typingMetrics.currentKey)}</strong>
              <span>{typingMetrics.activeFinger.label}</span>
            </article>
          </div>

          <div className="sim-flight-hud__bottom">
            <div className="sim-flight-card sim-flight-card--wide">
              <p className="sim-kicker">Objective</p>
              <h2>{missionView.objectiveTitle}</h2>
              <p>{missionView.objectiveSummary}</p>
            </div>
            <div className="sim-flight-controls">
              {HOME_ROW_CONTROL_GUIDE.map((control) => {
                const isActive =
                  (control.keyLabel === 'A' && controls.weightLeft) ||
                  (control.keyLabel === 'F' && controls.leftBrake) ||
                  (control.keyLabel === 'J' && controls.rightBrake) ||
                  (control.keyLabel === ';' && controls.weightRight) ||
                  (control.keyLabel === 'Space' && controls.speedBar)

                return (
                  <article
                    key={control.keyLabel}
                    className={`sim-flight-control${isActive ? ' is-active' : ''}`}
                  >
                    <strong>{control.keyLabel}</strong>
                    <span>{control.description}</span>
                  </article>
                )
              })}
            </div>
          </div>
        </section>
      )}
    </div>
  )
}

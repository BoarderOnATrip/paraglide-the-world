import { lazy, Suspense } from 'react'
import type { CSSProperties } from 'react'
import type { ActivityMode } from '../../data'
import type { MissionView } from '../../lib/mission-engine'
import type {
  TypingDerivedMetrics,
  TypingSessionState,
} from '../../lib/typing-engine'
import type { WorldDerivedMetrics } from '../../lib/world-engine'

const WorldMap = lazy(() =>
  import('../WorldMap').then((module) => ({
    default: module.WorldMap,
  })),
)

type FlightCardProps = {
  selectedActivityMode: ActivityMode
  metrics: TypingDerivedMetrics
  missionView: MissionView
  worldMetrics: WorldDerivedMetrics
  session: TypingSessionState
  flightStyle: CSSProperties
  isActiveFlight: boolean
  isPaused: boolean
}

export function FlightCard({
  selectedActivityMode,
  metrics,
  missionView,
  worldMetrics,
  session,
  flightStyle,
  isActiveFlight,
  isPaused,
}: FlightCardProps) {
  return (
    <section className="card card--flight">
      <div className="card__header">
        <div>
          <p className="card__eyebrow">
            {metrics.country.route} · {selectedActivityMode.name}
          </p>
          <h2>{metrics.country.name} World Pass</h2>
        </div>
        <p className="card__summary">{metrics.country.tagline}</p>
      </div>

      <div className="flight-stage" style={flightStyle}>
        <Suspense
          fallback={
            <div className="world-map world-map--loading">
              <div className="world-map__state">Loading world renderer...</div>
            </div>
          }
        >
          <WorldMap
            activityModeId={selectedActivityMode.id}
            country={metrics.country}
            checkpoints={missionView.checkpoints}
            progress={worldMetrics.travelProgress}
            isActiveFlight={isActiveFlight}
            isPaused={isPaused}
            windHeading={worldMetrics.windHeading}
            windSpeedKmh={worldMetrics.windSpeedKmh}
            speedKmh={worldMetrics.speedKmh}
          />
        </Suspense>
        <div className="flight-stage__sun" aria-hidden="true" />
        <div className="flight-stage__cloud flight-stage__cloud--one" />
        <div className="flight-stage__cloud flight-stage__cloud--two" />
        <div className="flight-stage__terrain flight-stage__terrain--far" />
        <div className="flight-stage__terrain flight-stage__terrain--near" />

        <div className="route-line" aria-hidden="true">
          {missionView.checkpoints.map((checkpoint) => {
            const markerProgress = Math.min(checkpoint.progress, 0.995)

            return (
              <div
                key={checkpoint.id}
                className={`route-line__marker${
                  checkpoint.status === 'cleared'
                    ? ' is-cleared'
                    : checkpoint.status === 'active'
                      ? ' is-active'
                      : ''
                }`}
                style={{ left: `${markerProgress * 100}%` }}
              >
                <span>{checkpoint.label}</span>
              </div>
            )
          })}
        </div>

        <div className="flight-stage__badge">
          <span>{selectedActivityMode.movementStyle}</span>
        </div>

        <div className="flight-stage__badge flight-stage__badge--secondary">
          <span>
            {isPaused
              ? 'Flight paused. Route position is held while the trusted hangout deck is open.'
              : isActiveFlight
                ? metrics.country.focus
                : `${selectedActivityMode.learningRole} Wind: ${worldMetrics.windLabel}.`}
          </span>
        </div>

        <div className="glider" aria-hidden="true">
          <div className="glider__wing" />
          <div className="glider__lines" />
          <div className="glider__pilot" />
        </div>
      </div>

      <div
        className={`typing-console typing-console--${session.lastInput}${
          isActiveFlight ? '' : ' is-resting'
        }${isPaused ? ' is-paused' : ''}`}
      >
        <div className="typing-console__header">
          <div>
            <p className="card__eyebrow">
              {isPaused
                ? 'Hangout Pause'
                : isActiveFlight
                  ? 'Active Flight'
                  : 'Scenic World Mode'}
            </p>
            <h3>
              {isPaused
                ? 'Flight is paused while you text trusted contacts and plan the next leg.'
                : isActiveFlight
                  ? metrics.country.mission
                  : `${selectedActivityMode.name} keeps the world social while your hands rest.`}
            </h3>
          </div>
          <div className="typing-console__stats">
            <span>Status {isPaused ? 'Paused' : 'Live'}</span>
            <span>Streak {session.streak}</span>
            <span>Best {session.bestStreak}</span>
            <span>Mistakes {session.mistakes}</span>
            <span>Speed {worldMetrics.speedKmh} km/h</span>
          </div>
        </div>

        {isPaused ? (
          <>
            <div className="typing-console__resting">
              <strong>Hangout hold</strong>
              <p>
                Your route position is locked while the chat deck is active.
                Resume when you want finger discipline feeding lift again.
              </p>
            </div>

            <p className="typing-console__hint">
              Pause gives the typing system a clean boundary. No flight keys are
              captured while you message trusted contacts.
            </p>
          </>
        ) : isActiveFlight ? (
          <>
            <div className="typing-console__stream" aria-live="polite">
              <span className="typing-console__stream--done">
                {metrics.completedText}
              </span>
              <span className="typing-console__stream--current">
                {metrics.currentKey === ' '
                  ? 'SPACE'
                  : metrics.currentKey.toUpperCase()}
              </span>
              <span className="typing-console__stream--upcoming">
                {metrics.upcomingText}
              </span>
            </div>

            <p className="typing-console__hint">
              Type the highlighted key to hold lift and keep the wing stable.
              The right finger movement is the control model. Clean technique
              feeds lift into the world simulation.
            </p>
          </>
        ) : (
          <>
            <div className="typing-console__resting">
              <strong>{selectedActivityMode.name}</strong>
              <p>{selectedActivityMode.summary}</p>
            </div>

            <p className="typing-console__hint">
              You only need active flying when you feel like it. Scenic modes
              keep the world explorable, social, and context-rich between skill
              runs.
            </p>
          </>
        )}
      </div>
    </section>
  )
}

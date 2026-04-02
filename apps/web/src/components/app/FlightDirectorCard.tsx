import type { ActivityMode } from '../../data'
import type { WorldDerivedMetrics } from '../../lib/world-engine'

type FlightDirectorCardProps = {
  selectedActivityMode: ActivityMode
  worldMetrics: WorldDerivedMetrics
  isPaused: boolean
  journeys: number
}

export function FlightDirectorCard({
  selectedActivityMode,
  worldMetrics,
  isPaused,
  journeys,
}: FlightDirectorCardProps) {
  return (
    <section className="card">
      <div className="card__header">
        <div>
          <p className="card__eyebrow">Flight Director</p>
          <h2>{worldMetrics.moodLabel}</h2>
        </div>
        <div className="coach-key">{isPaused ? 'PAUSE' : worldMetrics.windSpeedKmh}</div>
      </div>

      <p className="card__summary">
        {isPaused
          ? 'Flight is paused. Wind, lift, and travel are held while the trusted-contact deck is open.'
          : `${selectedActivityMode.name} is currently reading as ${worldMetrics.windLabel}. Heading ${worldMetrics.windHeading} degrees with ${worldMetrics.turbulencePercent}% turbulence.`}
      </p>

      <div className="director-grid">
        <article className="director-grid__item">
          <strong>Travel</strong>
          <span>{worldMetrics.travelPercent}% of current loop</span>
        </article>
        <article className="director-grid__item">
          <strong>Lift</strong>
          <span>{worldMetrics.liftPercent}% energy</span>
        </article>
        <article className="director-grid__item">
          <strong>Altitude</strong>
          <span>{worldMetrics.altitudeMeters} m</span>
        </article>
        <article className="director-grid__item">
          <strong>Journeys</strong>
          <span>{journeys} completed runs</span>
        </article>
      </div>
    </section>
  )
}

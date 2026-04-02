import type { CSSProperties } from 'react'
import { ACTIVITY_MODES, type ActivityMode, type ActivityModeId } from '../../data'
import { COUNTRIES, type CountryId } from '../../lib/typing-content'

type TopControlsProps = {
  selectedActivityMode: ActivityMode
  accuracy: number
  windSpeedKmh: number
  distanceKm: number
  selectedCountryId: CountryId
  onActivityModeSelect: (activityModeId: ActivityModeId) => void
  onCountrySelect: (countryId: CountryId) => void
}

export function TopControls({
  selectedActivityMode,
  accuracy,
  windSpeedKmh,
  distanceKm,
  selectedCountryId,
  onActivityModeSelect,
  onCountrySelect,
}: TopControlsProps) {
  return (
    <>
      <header className="hero">
        <p className="eyebrow">open-world typing flight platform</p>
        <h1>Paraglide the World</h1>
        <p className="hero__copy">
          Earth-scale travel, real-world mood, and accidental competence through
          movement. Paragliding is the high-skill mode. Scenic flights, boat
          tours, and car rides keep the world social when you want lower-pressure
          exploration.
        </p>

        <div className="metric-row">
          <article className="metric-card">
            <span className="metric-card__label">Mode</span>
            <strong>{selectedActivityMode.name}</strong>
          </article>
          <article className="metric-card">
            <span className="metric-card__label">Accuracy</span>
            <strong>{accuracy}%</strong>
          </article>
          <article className="metric-card">
            <span className="metric-card__label">Wind</span>
            <strong>{windSpeedKmh} km/h</strong>
          </article>
          <article className="metric-card">
            <span className="metric-card__label">Travel</span>
            <strong>{distanceKm} km</strong>
          </article>
        </div>
      </header>

      <section className="mode-strip" aria-label="Traversal modes">
        {ACTIVITY_MODES.map((mode) => {
          const isActive = mode.id === selectedActivityMode.id

          return (
            <button
              key={mode.id}
              className={`mode-chip${isActive ? ' is-active' : ''}`}
              onClick={() => {
                onActivityModeSelect(mode.id)
              }}
              type="button"
            >
              <span className="mode-chip__name">{mode.name}</span>
              <span className="mode-chip__summary">{mode.summary}</span>
            </button>
          )
        })}
      </section>

      <section className="country-strip" aria-label="Country routes">
        {COUNTRIES.map((country) => {
          const isActive = country.id === selectedCountryId

          return (
            <button
              key={country.id}
              className={`country-chip${isActive ? ' is-active' : ''}`}
              onClick={() => {
                onCountrySelect(country.id)
              }}
              type="button"
            >
              <span
                className="country-chip__flag"
                style={
                  {
                    '--flag-one': country.flagColors[0],
                    '--flag-two': country.flagColors[1],
                    '--flag-three': country.flagColors[2],
                  } as CSSProperties
                }
              />
              <span className="country-chip__text">
                <strong>{country.name}</strong>
                <span>
                  {country.region} · {country.route}
                </span>
              </span>
            </button>
          )
        })}
      </section>
    </>
  )
}

import type { TypingDerivedMetrics } from '../../lib/typing-engine'

type CountryRadioCardProps = {
  metrics: TypingDerivedMetrics
  travelProgress: number
  radioPlaying: boolean
  radioSupported: boolean
  onToggleRadio: () => void
}

export function CountryRadioCard({
  metrics,
  travelProgress,
  radioPlaying,
  radioSupported,
  onToggleRadio,
}: CountryRadioCardProps) {
  const activeFactIndex =
    metrics.country.facts.length === 0
      ? -1
      : Math.min(
          metrics.country.facts.length - 1,
          Math.floor(travelProgress * metrics.country.facts.length),
        )
  const activeFact =
    activeFactIndex >= 0
      ? metrics.country.facts[activeFactIndex] ?? metrics.country.facts[0] ?? ''
      : ''

  return (
    <section className="card">
      <div className="card__header">
        <div>
          <p className="card__eyebrow">Country Radio</p>
          <h2>{metrics.country.radioStation}</h2>
        </div>
        <button
          className="radio-toggle"
          disabled={!radioSupported}
          onClick={onToggleRadio}
          type="button"
        >
          {radioPlaying ? 'Stop Radio' : 'Play Radio'}
        </button>
      </div>

      <p className="card__summary">
        Now playing: {activeFact}
        {!radioSupported ? ' Browser speech synthesis is unavailable here.' : ''}
      </p>

      <div className="fact-list">
        {metrics.country.facts.map((fact, index) => (
          <article
            key={fact}
            className={`fact-list__item${index === activeFactIndex ? ' is-active' : ''}`}
          >
            {fact}
          </article>
        ))}
      </div>
    </section>
  )
}

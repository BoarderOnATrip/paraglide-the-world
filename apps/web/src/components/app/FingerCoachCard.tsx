import { FINGER_GUIDES } from '../../lib/typing-content'
import type { TypingDerivedMetrics } from '../../lib/typing-engine'

type FingerCoachCardProps = {
  metrics: TypingDerivedMetrics
  isTypingLive: boolean
  isPaused: boolean
  isActiveFlight: boolean
}

export function FingerCoachCard({
  metrics,
  isTypingLive,
  isPaused,
  isActiveFlight,
}: FingerCoachCardProps) {
  return (
    <section className="card">
      <div className="card__header">
        <div>
          <p className="card__eyebrow">Finger Coach</p>
          <h2>
            {isTypingLive ? `Next move: ${metrics.activeFinger.label}` : 'Hands at rest'}
          </h2>
        </div>
        <div className="coach-key">
          {isTypingLive
            ? metrics.currentKey === ' '
              ? 'SPACE'
              : metrics.currentKey.toUpperCase()
            : 'REST'}
        </div>
      </div>

      <p className="card__summary">
        {isPaused
          ? 'Typing input is intentionally suspended. This is the low-pressure window for trusted chat and route planning.'
          : isActiveFlight
            ? metrics.activeFinger.reminder
            : 'Paragliding teaches the precise finger mechanics. Scenic modes let you stay in the world without forcing constant key work while travel continues.'}
      </p>

      <div className="finger-grid">
        {FINGER_GUIDES.map((guide) => (
          <article
            key={guide.id}
            className={`finger-grid__item${
              isTypingLive && metrics.activeFinger.id === guide.id ? ' is-active' : ''
            }`}
          >
            <strong>{guide.label}</strong>
            <span>{guide.keys}</span>
          </article>
        ))}
      </div>
    </section>
  )
}

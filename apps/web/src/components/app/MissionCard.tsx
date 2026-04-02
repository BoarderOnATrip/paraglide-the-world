import type { MissionView } from '../../lib/mission-engine'

type MissionCardProps = {
  missionView: MissionView
}

export function MissionCard({ missionView }: MissionCardProps) {
  return (
    <section className="card">
      <div className="card__header">
        <div>
          <p className="card__eyebrow">Route Mission</p>
          <h2>{missionView.objectiveTitle}</h2>
        </div>
        <div className={`mission-badge mission-badge--${missionView.previewRating}`}>
          {missionView.previewLabel}
        </div>
      </div>

      <p className="card__summary">{missionView.objectiveSummary}</p>

      <div className="mission-progress">
        {missionView.checkpoints.map((checkpoint) => (
          <article
            key={checkpoint.id}
            className={`mission-progress__item is-${checkpoint.status}`}
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

      <div className="contact-rules mission-stats">
        <article className="contact-rule">
          <strong>Cleared</strong>
          <p>
            {missionView.clearedCount}/{missionView.totalCount} live objectives
          </p>
        </article>
        <article className="contact-rule">
          <strong>Runs</strong>
          <p>{missionView.runCount} completed loops</p>
        </article>
      </div>

      {missionView.lastResult ? (
        <div className={`mission-result mission-result--${missionView.lastResult.rating}`}>
          <div className="mission-result__header">
            <strong>{missionView.lastResult.title}</strong>
            <span>Run {missionView.lastResult.runNumber}</span>
          </div>
          <p>{missionView.lastResult.summary}</p>
          <div className="mission-result__stats">
            <span>{missionView.lastResult.accuracy}% accuracy</span>
            <span>{missionView.lastResult.mistakes} mistakes</span>
            <span>{missionView.lastResult.distanceKm} km</span>
          </div>
        </div>
      ) : null}
    </section>
  )
}

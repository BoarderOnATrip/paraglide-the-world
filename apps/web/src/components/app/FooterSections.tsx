import { ACTIVITY_MODES, type ActivityModeId } from '../../data'

type FooterSectionsProps = {
  selectedActivityModeId: ActivityModeId
}

export function FooterSections({ selectedActivityModeId }: FooterSectionsProps) {
  return (
    <section className="footer-grid">
      <section className="card">
        <div className="card__header">
          <div>
            <p className="card__eyebrow">World Activities</p>
            <h2>How the world stays alive</h2>
          </div>
        </div>

        <div className="principles">
          {ACTIVITY_MODES.map((mode) => (
            <article
              key={mode.id}
              className={`principle${
                mode.id === selectedActivityModeId ? ' is-active' : ''
              }`}
            >
              <strong>{mode.name}</strong>
              <p>{mode.learningRole}</p>
            </article>
          ))}
        </div>
      </section>

      <section className="card">
        <div className="card__header">
          <div>
            <p className="card__eyebrow">Delivery Track</p>
            <h2>What gets built next</h2>
          </div>
        </div>

        <div className="roadmap">
          <article className="roadmap__step">
            <strong>01</strong>
            <p>Provider-neutral world shell and extracted typing engine.</p>
          </article>
          <article className="roadmap__step">
            <strong>02</strong>
            <p>Renderer benchmark against open data and Google-quality 3D options.</p>
          </article>
          <article className="roadmap__step">
            <strong>03</strong>
            <p>Whole-world metadata, flags, and guided learning overlays.</p>
          </article>
          <article className="roadmap__step">
            <strong>04</strong>
            <p>Trusted-contact voice and scenic social sessions after safety controls.</p>
          </article>
        </div>
      </section>
    </section>
  )
}

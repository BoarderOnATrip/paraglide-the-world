import type { FormEvent } from 'react'
import {
  ACTIVITY_MODES,
  CONTACT_MODEL,
  SOCIAL_LAYERS,
  TRUSTED_CONTACTS,
  type ActivityMode,
  type ActivityModeId,
  type ChatThread,
} from '../../data'

type HangoutDeckCardProps = {
  selectedActivityMode: ActivityMode
  threads: ChatThread[]
  selectedThreadId: string
  draftMessage: string
  isPaused: boolean
  onPauseToggle: () => void
  onContactOpen: (contactId: string) => void
  onThreadSelect: (threadId: string) => void
  onResumeWithMode: (activityModeId: ActivityModeId) => void
  onDraftMessageChange: (value: string) => void
  onDraftFocus: () => void
  onSendMessage: () => void
}

export function HangoutDeckCard({
  selectedActivityMode,
  threads,
  selectedThreadId,
  draftMessage,
  isPaused,
  onPauseToggle,
  onContactOpen,
  onThreadSelect,
  onResumeWithMode,
  onDraftMessageChange,
  onDraftFocus,
  onSendMessage,
}: HangoutDeckCardProps) {
  const selectedThread =
    threads.find((thread) => thread.id === selectedThreadId) ?? threads[0] ?? null
  const selectedThreadParticipants =
    selectedThread == null
      ? []
      : TRUSTED_CONTACTS.filter((contact) =>
          selectedThread.participantIds.includes(contact.id),
        )
  const selectedThreadMode =
    ACTIVITY_MODES.find((mode) => mode.id === selectedThread?.modeHint) ??
    selectedActivityMode

  const handleSubmit = (event: FormEvent<HTMLFormElement>) => {
    event.preventDefault()
    onSendMessage()
  }

  return (
    <section className="card">
      <div className="card__header">
        <div>
          <p className="card__eyebrow">Hangout Deck</p>
          <h2>{isPaused ? 'Trusted chat live' : 'Trusted contact sessions'}</h2>
        </div>
        <button className="radio-toggle" onClick={onPauseToggle} type="button">
          {isPaused ? 'Resume Flight' : 'Pause for Chat'}
        </button>
      </div>

      <p className="card__summary">
        {CONTACT_MODEL.summary} The prototype keeps messaging inside trusted
        direct and invite-only group threads while public voice stays out of
        scope.
      </p>

      <div className="contact-rules">
        <article className="contact-rule">
          <strong>Entry</strong>
          <p>{CONTACT_MODEL.entryRule}</p>
        </article>
        <article className="contact-rule">
          <strong>Consent</strong>
          <p>{CONTACT_MODEL.consentRule}</p>
        </article>
      </div>

      <div className="hangout-grid">
        <div className="hangout-panel">
          <div className="hangout-panel__header">
            <p className="card__eyebrow">Trusted Contacts</p>
            <span>Click a contact to open their thread and pause cleanly.</span>
          </div>

          <div className="contact-roster">
            {TRUSTED_CONTACTS.map((contact) => {
              const threadId = threads.find((thread) =>
                thread.participantIds.includes(contact.id),
              )?.id
              const favoriteMode =
                ACTIVITY_MODES.find((mode) => mode.id === contact.favoriteMode)
                  ?.name ?? contact.favoriteMode
              const isSelected = threadId != null && selectedThread?.id === threadId

              return (
                <button
                  key={contact.id}
                  className={`contact-ping${isSelected ? ' is-active' : ''}`}
                  disabled={threadId == null}
                  onClick={() => {
                    onContactOpen(contact.id)
                  }}
                  type="button"
                >
                  <div className="contact-ping__header">
                    <strong>{contact.name}</strong>
                    <span>{contact.status}</span>
                  </div>
                  <p>{contact.presence}</p>
                  <small>Favorite mode: {favoriteMode}</small>
                </button>
              )
            })}
          </div>
        </div>

        <div className="hangout-panel">
          <div className="hangout-panel__header">
            <p className="card__eyebrow">Messages</p>
            <span>
              {selectedThread == null
                ? 'Choose a thread'
                : `${selectedThread.summary} · ${selectedThreadMode.name}`}
            </span>
          </div>

          <div className="thread-tabs">
            {threads.map((thread) => {
              const lastMessage = thread.messages[thread.messages.length - 1]

              return (
                <button
                  key={thread.id}
                  className={`thread-tab${
                    selectedThread?.id === thread.id ? ' is-active' : ''
                  }`}
                  onClick={() => {
                    onThreadSelect(thread.id)
                  }}
                  type="button"
                >
                  <strong>{thread.name}</strong>
                  <span>{lastMessage?.text ?? thread.summary}</span>
                </button>
              )
            })}
          </div>

          {selectedThread ? (
            <div className="thread-stage">
              <div className="thread-stage__meta">
                <strong>{selectedThread.name}</strong>
                <span>{selectedThread.summary}</span>
              </div>

              <div className="thread-stage__participants">
                {selectedThreadParticipants.map((contact) => (
                  <span key={contact.id} className="thread-chip">
                    {contact.name}
                  </span>
                ))}
              </div>

              <div className="chat-log" aria-live="polite">
                {selectedThread.messages.map((message) => (
                  <article
                    key={message.id}
                    className={`chat-bubble${message.authorId === 'me' ? ' is-me' : ''}`}
                  >
                    <strong>{message.authorName}</strong>
                    <p>{message.text}</p>
                    <span>{message.timestamp}</span>
                  </article>
                ))}
              </div>

              <div className="thread-actions">
                <button
                  className="radio-toggle radio-toggle--secondary"
                  onClick={() => {
                    onResumeWithMode(selectedThread.modeHint)
                  }}
                  type="button"
                >
                  Fly {selectedThreadMode.name}
                </button>
                <button
                  className="radio-toggle radio-toggle--ghost"
                  onClick={onPauseToggle}
                  type="button"
                >
                  {isPaused ? 'Resume Route' : 'Pause Here'}
                </button>
              </div>

              <form className="chat-composer" onSubmit={handleSubmit}>
                <input
                  className="chat-composer__input"
                  onChange={(event) => {
                    onDraftMessageChange(event.target.value)
                  }}
                  onFocus={onDraftFocus}
                  placeholder={
                    isPaused
                      ? 'Type a message to trusted contacts'
                      : 'Focus here to pause and message'
                  }
                  type="text"
                  value={draftMessage}
                />
                <button
                  className="radio-toggle"
                  disabled={!selectedThread || !draftMessage.trim()}
                  type="submit"
                >
                  Send
                </button>
              </form>
            </div>
          ) : null}
        </div>
      </div>

      <div className="social-layers">
        {SOCIAL_LAYERS.map((layer) => (
          <article key={layer.id} className="social-layer">
            <strong>{layer.name}</strong>
            <span>{layer.visibility}</span>
            <p>{layer.summary}</p>
          </article>
        ))}
      </div>
    </section>
  )
}

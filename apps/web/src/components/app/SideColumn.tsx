import type { ActivityMode, ActivityModeId, ChatThread } from '../../data'
import { CountryRadioCard } from './CountryRadioCard'
import { FingerCoachCard } from './FingerCoachCard'
import { FlightDirectorCard } from './FlightDirectorCard'
import { HangoutDeckCard } from './HangoutDeckCard'
import { MissionCard } from './MissionCard'
import type { MissionView } from '../../lib/mission-engine'
import type { TypingDerivedMetrics } from '../../lib/typing-engine'
import type { WorldDerivedMetrics } from '../../lib/world-engine'

type SideColumnProps = {
  selectedActivityMode: ActivityMode
  journeys: number
  metrics: TypingDerivedMetrics
  worldMetrics: WorldDerivedMetrics
  missionView: MissionView
  isPaused: boolean
  isTypingLive: boolean
  isActiveFlight: boolean
  radioPlaying: boolean
  radioSupported: boolean
  draftMessage: string
  threads: ChatThread[]
  selectedThreadId: string
  onPauseToggle: () => void
  onToggleRadio: () => void
  onContactOpen: (contactId: string) => void
  onThreadSelect: (threadId: string) => void
  onResumeWithMode: (activityModeId: ActivityModeId) => void
  onDraftMessageChange: (value: string) => void
  onDraftFocus: () => void
  onSendMessage: () => void
}

export function SideColumn({
  selectedActivityMode,
  journeys,
  metrics,
  worldMetrics,
  missionView,
  isPaused,
  isTypingLive,
  isActiveFlight,
  radioPlaying,
  radioSupported,
  draftMessage,
  threads,
  selectedThreadId,
  onPauseToggle,
  onToggleRadio,
  onContactOpen,
  onThreadSelect,
  onResumeWithMode,
  onDraftMessageChange,
  onDraftFocus,
  onSendMessage,
}: SideColumnProps) {
  return (
    <aside className="side-column">
      <MissionCard missionView={missionView} />
      <FlightDirectorCard
        selectedActivityMode={selectedActivityMode}
        worldMetrics={worldMetrics}
        isPaused={isPaused}
        journeys={journeys}
      />
      <FingerCoachCard
        metrics={metrics}
        isTypingLive={isTypingLive}
        isPaused={isPaused}
        isActiveFlight={isActiveFlight}
      />
      <CountryRadioCard
        metrics={metrics}
        travelProgress={worldMetrics.travelProgress}
        radioPlaying={radioPlaying}
        radioSupported={radioSupported}
        onToggleRadio={onToggleRadio}
      />
      <HangoutDeckCard
        selectedActivityMode={selectedActivityMode}
        threads={threads}
        selectedThreadId={selectedThreadId}
        draftMessage={draftMessage}
        isPaused={isPaused}
        onPauseToggle={onPauseToggle}
        onContactOpen={onContactOpen}
        onThreadSelect={onThreadSelect}
        onResumeWithMode={onResumeWithMode}
        onDraftMessageChange={onDraftMessageChange}
        onDraftFocus={onDraftFocus}
        onSendMessage={onSendMessage}
      />
    </aside>
  )
}

export type Wingmate = {
  name: string
  role: string
  status: string
}

export type WorldCamera = {
  center: [number, number]
  zoom: number
  pitch: number
  bearing: number
}

export type CountryContent = {
  id: string
  name: string
  region: string
  route: string
  tagline: string
  radioStation: string
  focus: string
  mission: string
  lessonWords: string[]
  facts: string[]
  waypoints: string[]
  wingmates: Wingmate[]
  flagColors: [string, string, string]
  camera: WorldCamera
}

export type FingerGuide = {
  id: string
  label: string
  keys: string
  reminder: string
}

export type ActivityModeId =
  | 'paragliding'
  | 'scenic-flight'
  | 'boat-tour'
  | 'car-ride'

export type ActivityMode = {
  id: ActivityModeId
  name: string
  summary: string
  movementStyle: string
  learningRole: string
  isActiveFlight: boolean
  baseSpeedKmh: number
  maxBoostSpeedKmh: number
  typingInfluence: number
  autopilotStrength: number
  windExposure: number
  turbulenceFloor: number
}

export type SocialLayerId = 'open' | 'group' | 'direct'

export type SocialLayer = {
  id: SocialLayerId
  name: string
  summary: string
  relationshipModel: string
  visibility: string
  textSupported: boolean
  voiceSupported: boolean
}

export type ContactModel = {
  id: 'invite-only'
  name: string
  summary: string
  entryRule: string
  consentRule: string
}

export type TrustedContact = {
  id: string
  name: string
  status: string
  presence: string
  favoriteMode: ActivityModeId
}

export type ChatMessage = {
  id: string
  authorId: string
  authorName: string
  text: string
  timestamp: string
}

export type ChatThread = {
  id: string
  name: string
  kind: 'direct' | 'group'
  participantIds: string[]
  summary: string
  modeHint: ActivityModeId
  messages: ChatMessage[]
}

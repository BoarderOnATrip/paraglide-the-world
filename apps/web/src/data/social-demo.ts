import type { ChatThread, TrustedContact } from './content-types'

export const TRUSTED_CONTACTS: TrustedContact[] = [
  {
    id: 'mila',
    name: 'Mila',
    status: 'Online',
    presence: 'Setting up a scenic launch over Rio.',
    favoriteMode: 'scenic-flight',
  },
  {
    id: 'joao',
    name: 'Joao',
    status: 'In the air',
    presence: 'Testing crosswinds and thermal climbs.',
    favoriteMode: 'paragliding',
  },
  {
    id: 'aya',
    name: 'Aya',
    status: 'Available',
    presence: 'Collecting story notes for a city ride.',
    favoriteMode: 'car-ride',
  },
  {
    id: 'ridge-crew',
    name: 'Ridge Crew',
    status: 'Group ready',
    presence: 'Boat tour planning with three members.',
    favoriteMode: 'boat-tour',
  },
]

export const INITIAL_CHAT_THREADS: ChatThread[] = [
  {
    id: 'direct-mila',
    name: 'Mila',
    kind: 'direct',
    participantIds: ['mila'],
    summary: 'Trusted direct contact',
    modeHint: 'scenic-flight',
    messages: [
      {
        id: 'mila-1',
        authorId: 'mila',
        authorName: 'Mila',
        text: 'If you pause over Rio, I can talk you through the coastal route.',
        timestamp: '11:20',
      },
      {
        id: 'me-1',
        authorId: 'me',
        authorName: 'You',
        text: 'Perfect. I want the flight to feel social without breaking focus.',
        timestamp: '11:22',
      },
    ],
  },
  {
    id: 'group-ridge',
    name: 'Ridge Crew',
    kind: 'group',
    participantIds: ['joao', 'aya', 'ridge-crew'],
    summary: 'Invite-only group channel',
    modeHint: 'boat-tour',
    messages: [
      {
        id: 'group-1',
        authorId: 'ridge-crew',
        authorName: 'Ridge Crew',
        text: 'Boat tour first, then an evening paraglide when the wind settles.',
        timestamp: '11:18',
      },
      {
        id: 'group-2',
        authorId: 'aya',
        authorName: 'Aya',
        text: 'I can switch to a city ride afterward and keep the chat open.',
        timestamp: '11:23',
      },
    ],
  },
]

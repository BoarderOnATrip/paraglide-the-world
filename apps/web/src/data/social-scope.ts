import type { ContactModel, SocialLayer } from './content-types'

export const CONTACT_MODEL: ContactModel = {
  id: 'invite-only',
  name: 'Invite-only contact',
  summary:
    'Players only become direct hangout contacts through explicit invites and mutual consent.',
  entryRule: 'A relationship must be formed before direct hangouts or direct messages are enabled.',
  consentRule:
    'Each side must explicitly accept before a social layer is opened between them.',
}

export const SOCIAL_LAYERS: SocialLayer[] = [
  {
    id: 'direct',
    name: 'Direct',
    summary: 'One-to-one contact for private messages and focused hangouts.',
    relationshipModel: 'Invite-only contact chain',
    visibility: 'Private',
    textSupported: true,
    voiceSupported: true,
  },
  {
    id: 'group',
    name: 'Group',
    summary: 'Small invited groups for shared scenic flights, boat tours, or car rides.',
    relationshipModel: 'Invite-only party membership',
    visibility: 'Members only',
    textSupported: true,
    voiceSupported: true,
  },
  {
    id: 'open',
    name: 'Open',
    summary: 'Future public channels and open rooms for broader world participation.',
    relationshipModel: 'Public presence with moderation',
    visibility: 'Public',
    textSupported: true,
    voiceSupported: true,
  },
]

export const SOCIAL_SCOPE = {
  contactModel: CONTACT_MODEL,
  layers: SOCIAL_LAYERS,
  notes: [
    'Open voice is a future surface, not a launch requirement for the core flight loop.',
    'Private and group hangouts should be available before public open channels.',
    'Text chat can be layered alongside scenic modes when the product needs lower-pressure communication.',
  ],
} as const

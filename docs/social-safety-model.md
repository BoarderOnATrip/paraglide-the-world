# Social Safety Model

## Product direction

The social model now has four important constraints:

- under-18 users are allowed
- hangouts should be based on trusted prior contact or mutual approval
- open mic is preferred for natural conversation
- public open channels remain part of the long-term vision

These constraints are not compatible with an immediate public-open-voice launch. The system must stage social access.

## Recommended implementation model

### Stage 1: Trusted-contact only

- users can only start live hangouts with explicitly approved contacts
- approval should be done through invite code exchange, mutual accept flow, or account link approval
- direct voice can be open mic within that trusted session
- private group sessions can inherit open mic if every participant is approved by the host or group rules

### Stage 2: Structured groups

- recurring groups
- host controls
- join permissions
- mute, block, report, remove, and leave tools

### Stage 3: Public spaces

- open channels
- discoverable rooms
- public proximity or room audio

This stage should not ship until moderation, privacy, logging, appeals, and age-policy handling are explicit.

## Why not literal phone-number exchange

The product intent is clear: players should not freely encounter strangers and immediately enter voice sessions.

The implementation should preserve that intent without making phone numbers part of the core product graph.

Safer equivalents:

- invite codes exchanged offline
- QR or link-based friend approval
- mutual handle approval

## Minimum controls for any live audio

- mute
- block
- report
- leave instantly
- host remove or kick for private groups
- account-level privacy settings

## Compliance note

Because under-18 users are in scope, live communication and data collection need formal privacy and safety review before public rollout.

Relevant official source:

- [FTC COPPA overview and rulemaking materials](https://www.ftc.gov/business-guidance/privacy-security/childrens-privacy)

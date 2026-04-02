# Decision Log

## How to use this file

Each item lists:

- status
- decision
- why it matters
- implementation note

## Decision register

### D-001 Launch boundary

Status:

- provisional

Decision:

- the north star includes open, group, and direct communication
- the first shippable build should still exclude public social systems
- the first shippable build should focus on solo world traversal plus progression

Why it matters:

- this determines whether the team is shipping a game first or a live social platform immediately

Implementation note:

- treat solo open-world flight as Scope A
- treat social as Scope B and Scope C
- do not merge public voice into the first shippable build without a safety stack

### D-002 World presentation

Status:

- locked

Decision:

- full 3D geospatial flight is the target
- provider remains neutral until evaluated

Why it matters:

- this drives the renderer, asset pipeline, and performance budget

Implementation note:

- evaluate Google Photorealistic 3D Tiles plus Cesium against more open alternatives before hard commitment

### D-003 Camera freedom

Status:

- locked

Decision:

- fly-anywhere traversal is the target direction

Why it matters:

- this removes the simpler route-only implementation path

Implementation note:

- teaching loops will need to coexist with open exploration, likely via missions, overlays, challenges, and adaptive lesson prompts rather than fixed corridors

### D-004 Typing curriculum

Status:

- partial

Decision:

- English QWERTY first
- target player is broad: anyone seeking dexterity and fun
- active skill teaching happens mainly in paragliding mode
- other movement modes can exist as lower-intensity exploration or hangout modes

Why it matters:

- the entire learning system depends on this

Implementation note:

- still need explicit lesson tiers, mastery thresholds, and adaptive difficulty rules

### D-005 Progression model

Status:

- open

Why it matters:

- progression controls retention and content pacing

Implementation note:

- open-world traversal removes country unlocks as the main gate
- progression likely needs mastery tracks, cosmetics, missions, certifications, and social status layers instead

### D-006 Country content scope

Status:

- provisional

Decision:

- the entire world is in scope as the content surface
- start with flags, stats, and facts sourced from structured external references

Why it matters:

- global content changes the problem from authored route packs to metadata ingestion and curation

Implementation note:

- use automated country metadata first
- layer authored radio and richer place storytelling later
- do not assume Wikipedia text can simply be copied verbatim into product surfaces

### D-007 Platform target

Status:

- locked

Decision:

- desktop web first

Why it matters:

- typing-first controls strongly favor desktop

### D-008 Social scope

Status:

- partial and conflicted

Decision:

- long-term direction includes open, group, and direct channels
- paused text chat is in scope
- access to hangout sessions should be based on trusted prior contact or mutual approval
- public social systems should not be part of the first shippable build

Why it matters:

- this is the primary driver of backend, moderation, and privacy complexity

Implementation note:

- trusted-contact rooms can be scheduled earlier
- public rooms and open channels must stay behind safety gates

### D-009 Voice model

Status:

- partial

Why it matters:

- voice changes legal, safety, and moderation requirements immediately

Decision:

- open mic is preferred by product direction
- voice should initially exist only in trusted direct or private group contexts

Implementation note:

- do not enable public open mic while under-18 users are allowed
- still need explicit distinction between public, group, and direct audio

### D-010 Age and safety policy

Status:

- partial

Why it matters:

- this can become a launch blocker

Decision:

- under-18 users are allowed in the product

Implementation note:

- this requires a real trust-and-safety and privacy review before public communication ships
- offline-shared invite codes or mutual friend approval are safer than requiring phone-number exchange in product

### D-011 Map provider

Status:

- provisional

Decision:

- stay provider-neutral for now
- evaluate options against quality, openness, and cost

Why it matters:

- licensing, attribution, quota, and spend profile must be acceptable before committing

Implementation note:

- Google Earth API is no longer available; if Google-quality 3D is desired on the web, the current official path is Google Maps Platform Photorealistic 3D Tiles rendered through a compatible 3D renderer

### D-012 Commercial constraints

Status:

- partial

Decision:

- prefer free when time is acceptable
- pay when speed or capability justifies it

Why it matters:

- architecture choices depend on budget

Implementation note:

- still need actual ceilings for maps, hosting, voice, and moderation once vendor options are modeled

# Execution Roadmap

## Objective

Ship Paraglide the World as a real product, not a permanent prototype.

The vision is now larger than the original MVP:

- open-world global traversal
- real-world geography and terrain
- typing taught through flight mechanics
- global social spaces with open, group, and direct communication
- user-added contextual layers over time

That is a platform roadmap, not a single milestone. The only credible path is phased:

1. build the open-world solo flight core
2. validate the real-world renderer and data stack
3. add structured social layers with safety controls
4. add creator and world-building layers after the network and policy foundations are stable

## Locked direction from user

- desktop web first
- English QWERTY first
- provider-neutral renderer evaluation for now
- open fly-anywhere world traversal is the north star
- real terrain and wind dynamics matter
- social should ultimately support open, group, and direct channels
- under-18 users are allowed
- hangout access should be rooted in prior real-world contact or mutual approval
- open mic is preferred inside trusted sessions
- scenic travel modes beyond active paragliding are in scope
- text messaging while paused is in scope
- whole-world coverage is the long-term content surface

## Execution interpretation

To keep this finishable, the team will treat the product as three concentric scopes:

### Scope A: First shippable build

- desktop web
- open-world solo traversal
- real-world globe or terrain presentation
- typing-driven flight mechanics
- multiple traversal modes, with paragliding as the main skill mode
- country metadata, flags, and basic facts
- saved progress

### Scope B: Social beta

- trusted-contact sessions
- direct voice and group voice
- paused text chat
- presence and invite flow
- mute, block, and report

### Scope C: World platform

- open channels
- public rooms
- user-added contextual media
- community moderation and trust tooling
- richer creator content layers

Public voice and user-generated world layers are not removed. They are simply not allowed to collapse the first shippable build.

## Finish definition

For project-management purposes, “finish” now means:

1. Scope A shipped as a strong open-world desktop experience.
2. Scope B designed and sequenced with explicit safety controls.
3. Scope C kept behind policy and moderation gates until the system can safely support it.

## Workstreams

### 1. Product and curriculum

Owns:

- lesson structure
- progression
- player skill levels
- accuracy rules
- reward loops

Definition of done:

- complete typing curriculum spec
- open-world lesson and progression spec
- scoring and mastery model

### 2. Client gameplay

Owns:

- flight HUD
- lesson runner
- globe camera and flight presentation
- traversal mode switching
- results flow
- settings and onboarding

Definition of done:

- playable world traversal loop
- responsive keyboard input
- polished onboarding and results

### 3. World and rendering

Owns:

- provider evaluation
- globe and terrain renderer
- wind and terrain integration points
- map provider validation

Definition of done:

- world renderer decision documented
- production-safe global rendering path chosen

### 4. Backend and progression

Owns:

- auth
- save data
- telemetry
- progression persistence
- session state
- social graph foundations

Definition of done:

- stable account and save flow
- event model for accuracy, streaks, lesson completion, and unlocks

### 5. Content pipeline

Owns:

- country metadata ingestion
- fact sourcing
- flag and stats pipeline
- radio voice workflow
- content review and fact checking

Definition of done:

- world metadata pipeline operational
- repeatable workflow for adding richer authored content later

### 6. Social and safety

Owns:

- friend model
- room access policy
- voice chat rules
- mute, block, and report systems
- privacy and age gating

Definition of done:

- explicit policy before any live public communication ships

## Milestones

### Milestone 0: Concept lock

- finalize Scope A boundary
- choose target platform and device floor
- confirm keyboard layout and target audience
- define safety posture for future social layers

### Milestone 1: Core loop alpha

- deterministic typing engine
- open-world flight sandbox
- scoring and streak model
- finger coach
- wind and glide fundamentals
- traversal mode framework

### Milestone 2: World vertical slice

- whole-world traversal
- country metadata and flags
- fact and radio baseline
- scenic travel modes
- profile and progression
- polished visual identity
- replayable session flow

### Milestone 3: Production beta

- persistence
- analytics
- performance pass
- content ingestion tooling
- tutorial and settings
- contact and invite model foundation

### Milestone 4: Social beta

- trusted-contact rooms
- direct and group voice
- paused text chat
- reporting and blocking

### Milestone 5: Public world systems

- open channels
- moderated public presence
- publishable contextual media
- creator permissions and moderation queues

### Milestone 6: Provider and scale hardening

- final renderer/provider decision
- cost and licensing validation
- performance and browser baseline tests
- scale and observability review

## Hard gates

Do not start full implementation of these until they are explicitly approved:

- public voice chat
- minors in social spaces
- commercial dependence on a map provider without a spend model
- user-generated radio or persistent user content

## Safety interpretation

Because under-18 users are allowed, the team should treat public live communication as gated, not assumed.

- open mic can exist inside trusted direct or private group sessions
- public voice should remain blocked until moderation and policy are explicit
- “ask for your number in real life” should be implemented as an offline-exchanged invite code or mutual friend approval flow, not as a requirement to share phone numbers in product

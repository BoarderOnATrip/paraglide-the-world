# Technical Architecture

## Recommendation

Build the project as a web-first game with a staged renderer strategy.

### Foundation

- Client: React 19, TypeScript, Vite
- Prototype rendering: DOM and CSS for the earliest loop tests
- Production world rendering spike: CesiumJS-based globe or terrain rendering
- Typing engine: local deterministic lesson runner with per-key telemetry
- Radio prototype: browser speech synthesis over curated country facts

### Later production systems

- Progress and accounts: Postgres-backed app service
- Social voice: LiveKit or equivalent room-based WebRTC service
- Multiplayer state: authoritative session service for route rooms, ghost data, and shared events

## Why this stack

### React and Vite

They are fast to iterate with, easy to host, and good for combining a game HUD with product surfaces like onboarding, profile, and lesson management.

### Cesium-style rendering for the world layer

The project needs a camera that can travel over real geography, not only a flat slippy map. A 3D geospatial renderer is a better fit than trying to force a traditional map widget into a flight game.

Current Google Maps Tile API documentation explicitly supports Photorealistic 3D Tiles with CesiumJS and other 3D renderers, which makes a Cesium-based spike the most defensible way to test real-world flight. Official sources:

- [Photorealistic 3D Tiles](https://developers.google.com/maps/documentation/tile/3d-tiles)
- [Work with a 3D Tiles renderer](https://developers.google.com/maps/documentation/tile/use-renderer)

## Mapping reality check

Do not lock the game to “Google Maps” as a broad concept before validating the exact API and terms.

Current official Google Maps Platform documentation says:

- Map Tiles API use requires billing and quota management.
- Photorealistic 3D Tiles are available for custom renderers.
- attribution requirements must be preserved
- Maps Platform license restrictions still apply

Relevant official sources:

- [Map Tiles API overview](https://developers.google.com/maps/documentation/tile/)
- [Map Tiles API usage and billing](https://developers.google.com/maps/documentation/tile/usage-and-billing)
- [Map Tiles API policies](https://developers.google.com/maps/documentation/tile/policies)
- [Google Maps Platform Terms of Service](https://cloud.google.com/maps-platform/terms)

Practical implication:

- prototype the gameplay loop without coupling it to a specific map vendor
- run a short geospatial spike with CesiumJS plus Google 3D Tiles or an alternative imagery provider
- decide the long-term map provider only after validating cost, controls, attribution, and game-specific restrictions

## Gameplay systems

### Typing engine

Needs to own:

- lesson queues
- key normalization
- finger mapping
- error handling and streak logic
- adaptive difficulty rules
- scoring and telemetry

This should remain deterministic and testable outside the renderer.

### Flight model

The earliest version can be intentionally simple:

- accuracy raises lift
- streaks improve glide efficiency
- mistakes introduce drag and wobble
- rhythm matters more than twitch speed

Later, wind, thermals, descent, and route hazards can sit on top of the typing model.

### Country radio

Prototype with curated facts and browser speech synthesis.

Production version likely needs:

- authored country scripts
- factual review pipeline
- TTS provider for better voice quality
- locale support
- moderation rules for user-shared content if social radio becomes a feature

### Social layer

Voice and co-op flight should be separate services conceptually.

- voice rooms: low-latency audio and participant presence
- game rooms: route state, ghost positions, cooperative checkpoints, and scoring

LiveKit’s current room and track model is a strong fit for the audio layer because participants can publish microphone tracks and selectively subscribe to room media. Official source:

- [LiveKit track management](https://docs.livekit.io/intro/basics/rooms-participants-tracks/tracks/)

## Phased build plan

### Phase 0: prove the fantasy

- build a single-screen prototype
- validate that typing actually feels like flight
- confirm the visual tone

### Phase 1: single-player vertical slice

- clean HUD and onboarding
- country selection
- route progression
- results screen
- persistent progress

### Phase 2: world renderer spike

- integrate CesiumJS or equivalent
- test route cameras, terrain readability, and performance
- validate map licensing, attribution, and spend model

### Phase 3: country radio system

- authored scripts
- better TTS
- content tooling

### Phase 4: social flight

- party rooms
- positional or room voice
- shared route events
- moderation and reporting

## Immediate next engineering moves

1. Keep the current repo focused on the single-player loop.
2. Extract the typing engine into pure logic once the prototype interaction feels right.
3. Run a separate Cesium spike before any large content production.
4. Delay full multiplayer until single-player retention feels strong.

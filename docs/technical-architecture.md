# Technical Architecture

## Recommendation

Build the project in stages with a hard separation between:

- the current prototype sandbox
- a portable flight simulation core
- a future production renderer aimed at a beautiful full-render flight experience

The current repo should be treated as a fast iteration environment, not as proof that the final engine or renderer has already been chosen.

### Current prototype foundation

- Client sandbox: React 19, TypeScript, Vite
- Current world spike: CesiumJS plus geospatial tiles for terrain, camera, and scale validation
- Flight simulation: deterministic local loop that can be tuned outside the renderer
- Content and mission layer: lightweight local data and UI for iteration

### Target production direction

- Renderer target: Gaussian-splat-style or equivalent high-fidelity full-render world presentation
- Camera target: strong third-person chase camera with readable canopy motion and terrain speed
- Atmosphere target: layered air-mass model for wind, ridge lift, lee sink, thermals, turbulence, and landing energy
- Product priority: make solo flight feel and world presence convincing before expanding social or platform systems

### Later production systems

- Progress and accounts: Postgres-backed app service
- Social voice: LiveKit or equivalent room-based WebRTC service
- Multiplayer state: authoritative session service for route rooms, ghost data, and shared events

## Why this stack

### Current web sandbox

React and Vite are still useful for quick iteration on HUD, telemetry, tuning controls, and trying out world presentation ideas. They are not the final visual target. They are the fastest place to separate what belongs to simulation code from what belongs to a renderer or engine.

### Renderer direction

The project now wants more than a map-backed prototype. It wants terrain presence, atmospheric scale, and a rendering style that can plausibly feel like a true full-render flight sim instead of a HUD floating over a map surface.

That shifts the direction toward a Gaussian-splat-style or equivalent high-fidelity renderer. The current Cesium path is still valuable, but only as a near-term terrain and camera validation spike.

### Cesium spike rationale

The project still needs a camera that can travel over real geography, not only a flat slippy map. A 3D geospatial renderer remains a defensible way to test real-world flight before the final renderer is locked.

Current Google Maps Tile API documentation explicitly supports Photorealistic 3D Tiles with CesiumJS and other 3D renderers, which makes a Cesium-based spike a practical short-term validation path. Official sources:

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
- use the current Cesium path to validate scale, terrain readability, camera motion, and prototype data plumbing
- decide the long-term renderer and map provider only after validating visual quality, controls, attribution, spend, and game-specific restrictions

## Gameplay systems

### Input and skill layer

If the typing layer remains in the product, it should own:

- lesson queues
- key normalization
- finger mapping
- error handling and streak logic
- adaptive difficulty rules
- scoring and telemetry

This system should remain deterministic and testable outside the renderer, and it should not define the core flight-physics architecture.

### Flight model

The flight model should be built as a tunable simulation layer, not as a giant attempt at full CFD.

### First-pass simulation scope

V1 should own:

- wing state: position, heading, bank, pitch, airspeed, turn rate, sink rate, glide state, and ground clearance
- pilot inputs: left and right brake, symmetric brake, weight shift, speed bar, and flare intent
- air mass contributions: base sink, ridge lift, lee sink, thermal core lift, thermal edge decay, ring sink, wind advection, and turbulence gusts
- flight phases: launch, soaring, approach, flare, landed, and crashed
- deterministic stepping and replayable tests outside the renderer

Calibration priority:

- readable, believable, stable flight feel
- pilot intuition over strict CFD purity
- clear separation between aerodynamic tuning and world rendering

### Physics research shortlist

Do not try to absorb every physics reference at once. The useful shortlist is:

- glide polar, brake response, and canopy energy retention
- ridge lift, lee sink, and rotor behavior near terrain
- thermals, convection, and drift with changing wind
- wind gradient, shear, turbulence, and boundary-layer effects
- flare timing, landing energy management, and ground effect

### Physics reference workflow

Large external references should be mined selectively. The local `references/the_well` clone is useful as a research source, but only for narrow questions.

Most relevant starting points:

- `rayleigh_benard`: convection structure, buoyancy-driven circulation, and thermal-cell intuition
- `shear_flow`: shear-layer behavior, instability, and velocity-field structure
- `planetswe`: large-scale flow behavior, periodic forcing, and terrain-scale atmospheric patterns

Usage policy:

- start from dataset README and docs, not full data ingestion
- extract analogies and parameter intuition, not real-time solver code
- only go deeper into generation code or data layout when a specific mechanic needs it

Later layers can add:

- altitude-dependent wind gradient
- stronger rotor and terrain shadowing
- asymmetric canopy responses and surge
- site-specific weather presets
- broader failure and recovery cases once the baseline feels trustworthy

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
- validate that the wing feels good in the air
- confirm the visual tone and sense of scale

### Phase 1: single-player flight vertical slice

- launch, soaring, approach, flare, and landing loop
- tuned first site with readable wind and lift behavior
- clean HUD, telemetry, and replayable tuning scenarios

### Phase 2: full-render world spike

- evaluate Gaussian-splat-style or equivalent render approaches
- keep Cesium or similar geospatial tooling as a comparison spike, not an assumption
- test chase cameras, terrain readability, atmosphere depth, and performance
- validate map licensing, attribution, and spend model where relevant

### Phase 3: world content systems

- authored scripts
- better TTS
- content tooling

### Phase 4: social flight

- party rooms
- positional or room voice
- shared route events
- moderation and reporting

## Immediate next engineering moves

1. Keep the current repo focused on solo flight feel and renderer separation.
2. Continue extracting flight and atmosphere code into pure, testable modules.
3. Write a V1 physics tuning sheet around a small number of known scenarios: ridge pass, thermal climb, glide transition, approach, and flare.
4. Use `references/the_well` selectively, starting with `rayleigh_benard`, `shear_flow`, and `planetswe`.
5. Delay large social and backend expansion until the solo flight loop and renderer direction are credible.

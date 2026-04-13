# Paraglide the World

Paraglide the World is moving beyond the earlier typing prototype and beyond a narrow map-rendering experiment. The current aim is a beautiful full-render paraglider experience with strong flight feel: large-scale terrain, Gaussian-splat-style world presentation, and air-mass physics that make ridge lift, thermals, sink, turbulence, and landing behavior feel readable and convincing.

The repository now contains:

- An early simulator shell in `apps/web` used to test flight feel, HUD ideas, input mapping, and world presentation.
- Product and architecture docs in `docs` that document the rendering, physics, progression, and social roadmap.

## Current direction

The current project direction is centered on:

- Gaussian-splat-style or equivalent high-fidelity world rendering
- a true full-render visual experience with stronger atmosphere, terrain presence, and sense of scale
- more serious paraglider physics, including lift bands, thermals, sink, flare, and wind interaction
- a third-person chase camera with readable canopy and terrain motion
- control schemes that can support both disciplined typing ideas and more direct flight handling as the project evolves

## Current repo state

This repository is still in an early transition phase:

- `apps/web` is the current prototype sandbox
- the prototype still uses CesiumJS and Google tiles as one active rendering experiment
- the renderer and engine are not locked yet
- the long-term visual and simulation target is now ahead of the current implementation

## Repository layout

- `apps/web`: current prototype and simulation sandbox
- `docs/game-design.md`: game loop, progression, and experience goals
- `docs/physics-tuning-sheet.md`: first-pass flight mechanics, tuning targets, and scenario checks
- `docs/technical-architecture.md`: stack direction, production risks, and build phases
- `docs/unreal-gaussian-splatting-ingest.md`: how to source and import the first Unreal Gaussian-splat destination
- `tools/download-public-gaussian-splat-sample.sh`: fetch a public no-login Gaussian splat sample for Unreal import testing

## Run locally

```bash
pnpm install
pnpm dev
```

For the current web prototype, create `apps/web/.env` from `apps/web/.env.example` and set:

```bash
VITE_GOOGLE_MAPS_API_KEY=your_google_maps_platform_key
```

Without that key, the simulator shell still loads, but the Google-backed world layer stays in setup mode.

## Product direction

The long-term product is bigger than the current prototype:

- real-world flight over richly rendered terrain
- beautiful visuals paired with grounded canopy and airflow behavior
- geography, traversal, and discovery layered into the act of flying
- room to keep or reshape the typing layer based on what best serves the flight experience
- shared flights and social discovery later, once the core solo experience is worth deepening

The main technical caution is that the current prototype path is not the same thing as the final production commitment. Google/Cesium-based rendering is still useful for early validation, but the project direction now leaves room for a more fully rendered Gaussian-splat-style world stack and a deeper physics model as the target sharpens.

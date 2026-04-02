# Paraglide the World

Paraglide the World is pivoting from a stylized typing prototype into a photorealistic paraglider simulator. The new target is a third-person flight experience over real terrain, with home-row controls that keep your fingers disciplined while you explore.

The repository now contains:

- A web-first simulator shell in `apps/web` using CesiumJS as the renderer foundation.
- Product and architecture docs in `docs` that document the geospatial, realism, and social roadmap.

## Current focus

The current vertical slice is being reoriented around:

- Google-photorealistic terrain as the world target
- a third-person chase camera
- believable basic paraglider physics
- home-row flight controls on `A`, `F`, `J`, `;`, and `Space`

## Repository layout

- `apps/web`: React and Vite prototype
- `docs/game-design.md`: game loop, progression, and experience goals
- `docs/technical-architecture.md`: stack recommendation, production risks, and build phases

## Run locally

```bash
pnpm install
pnpm dev
```

To stream Google photorealistic 3D Tiles, create `apps/web/.env` from `apps/web/.env.example` and set:

```bash
VITE_GOOGLE_MAPS_API_KEY=your_google_maps_platform_key
```

Without that key, the simulator shell still loads, but the world layer stays in setup mode.

## Product direction

The long-term product is bigger than the current prototype:

- real-world flight over geospatial terrain
- adaptive typing lessons mapped to actual finger placement
- country radio that teaches geography without breaking flow
- shared flights with voice and cooperative discovery

The main technical caution is the map layer. Current Google Maps Platform docs support custom 3D experiences through Photorealistic 3D Tiles and CesiumJS, but they also require billing, attribution, quota management, and compliance with Maps Platform license restrictions. That validation work is captured in `docs/technical-architecture.md`.

# Rendering Options

## Purpose

Keep the renderer provider-neutral until the team has validated:

- visual quality
- control of the camera and flight model
- licensing and attribution burden
- performance on target desktop hardware
- cost at scale

## Current official facts

### Google

- Google Earth API is no longer available.
- The current official web path for Google-quality 3D visuals is Google Maps Platform Photorealistic 3D Tiles through the Map Tiles API.
- Google explicitly supports third-party renderers such as CesiumJS and deck.gl for Photorealistic 3D Tiles.
- Google requires billing, quota management, and visible attribution when its map data is rendered.

Official sources:

- [Google Earth API status](https://developers.google.com/earth)
- [Map Tiles API overview](https://developers.google.com/maps/documentation/tile/overview)
- [Photorealistic 3D Tiles overview](https://developers.google.com/maps/documentation/tile/3d-tiles-overview)
- [Work with a 3D Tiles renderer](https://developers.google.com/maps/documentation/tile/use-renderer)
- [Map Tiles API policies](https://developers.google.com/maps/documentation/tile/policies)
- [Map Tiles API usage and billing](https://developers.google.com/maps/documentation/tile/usage-and-billing)

### Cesium

- CesiumJS is a mature 3D geospatial web engine.
- Cesium’s official learning material supports terrain, imagery, and custom geospatial data pipelines.
- Cesium ion can tile and host terrain and imagery, but CesiumJS can also work with data hosted elsewhere.

Official sources:

- [CesiumJS fundamentals](https://cesium.com/learn/cesiumjs-fundamentals/)
- [Cesium terrain workflow](https://cesium.com/learn/3d-tiling/ion-tile-terrain/)

### MapLibre

- MapLibre is an open-source mapping stack.
- MapLibre GL JS supports globe view and terrain-oriented examples, but it is still a different class of tool from a full Cesium-style 3D globe engine.

Official sources:

- [MapLibre about](https://maplibre.org/about)
- [MapLibre GL JS docs](https://maplibre.org/maplibre-gl-js/docs/)
- [MapLibre globe view](https://maplibre.org/roadmap/maplibre-gl-js/globe-view/)
- [MapLibre examples overview](https://maplibre.org/maplibre-gl-js/docs/examples/)

### Mapbox

- Mapbox is viable, but current official pricing is usage-based.
- That makes it relevant for evaluation, but not the default fast-or-free path.

Official source:

- [Mapbox pricing guide](https://docs.mapbox.com/accounts/guides/pricing/)

## Candidate stacks

### Option A: Google Photorealistic 3D Tiles + CesiumJS

Best for:

- highest-fidelity Earth-like presentation
- true 3D free-flight feel
- fastest path to “Google Earth-like” visuals on the web

Pros:

- strongest visual match to the vision
- official Google support for CesiumJS renderer path
- real 3D camera freedom

Cons:

- billing and quota exposure
- attribution and policy burden
- provider lock-in risk

Recommendation:

- use this as the premium fidelity benchmark during evaluation

### Option B: CesiumJS + open or mixed terrain and imagery sources

Best for:

- provider-neutral architecture
- open-world camera freedom
- progressive migration between data sources

Pros:

- best engine fit for real globe flight
- keeps data layer swappable
- supports custom terrain, imagery, and later user overlays

Cons:

- visual quality depends on chosen data stack
- more integration work than taking one opinionated vendor bundle

Recommendation:

- strongest strategic baseline if the project wants long-term flexibility

### Option C: MapLibre GL JS + terrain/globe stack

Best for:

- cheaper and more open early experiments
- lighter-weight map-first prototypes

Pros:

- open-source ecosystem
- globe and terrain capabilities exist
- good fit for hybrid map plus HUD surfaces

Cons:

- weaker fit for high-freedom 3D paragliding than Cesium-style rendering
- likely not enough alone for the full “Google Earth with flight” target

Recommendation:

- useful as a fallback or secondary prototype path, not the primary long-term bet

## Recommended evaluation sequence

1. Build a provider-neutral flight prototype API in the client.
2. Run a CesiumJS spike with placeholder or open data first.
3. Run a Google Photorealistic 3D Tiles spike as the premium benchmark.
4. Compare fidelity, controls, performance, attribution burden, and estimated spend.
5. Lock the production renderer only after those tests.

## Current recommendation

Build the game around a renderer abstraction, then benchmark:

- baseline: CesiumJS with provider-neutral data plumbing
- premium benchmark: Google Photorealistic 3D Tiles through CesiumJS

That keeps the codebase aligned with your vision without committing prematurely to a paid or policy-heavy provider.

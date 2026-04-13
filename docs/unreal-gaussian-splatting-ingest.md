# Unreal Gaussian Splatting Ingest

This is the first practical path for getting a real Gaussian-splat destination into `ParaglideTheWorldUE`.

## Best first asset source on Mac

Use `Polycam` as the first asset source.

Why:

- Polycam has an official Unreal workflow for Gaussian Splats.
- Their workflow explicitly separates `PLY` render data from `GLTF` collision data, which matches this game's architecture.
- It is the cleanest path for a real destination where visuals and gameplay surfaces stay separate.

Official reference:

- `https://learn.poly.cam/hc/en-us/articles/42506758152084-Export-Gaussian-Splats-from-Polycam-to-Unreal-Engine`

## Plugin path

### Current repo-integrated plugin path on Mac

The repo now vendors `NanoGS` directly under `ParaglideTheWorldUE/Plugins/NanoGS`.

Reason:

- It is source-available and builds in the current `UE 5.7` Mac project.
- It imports `.ply` Gaussian splats into a native `GaussianSplatAsset`.
- It gives the project a zero-login, in-repo test path for Gaussian destination work.

Practical outcome:

- `tools/import-unreal-gaussian-splat-sample.sh` imports a public sample `.ply` into Unreal.
- `tools/create-unreal-gaussian-destination-pack.sh` wraps that imported asset in a `UParaglideDestinationPack`.

Important note:

- `NanoGS` currently gives us a clean render-path spike, not a full production world stack.
- We still need collision proxies and authored airflow metadata for real destinations.

### Secondary plugin path to evaluate

`UEGaussianSplatting` is still worth evaluating.

Reason:

- Its public documentation explicitly says the plugin supports `Windows`, `Mac`, and `Linux`.
- It supports direct `.ply` import into Unreal.
- It also documents collision generation, which is useful even if we still prefer separate proxy meshes for authoritative gameplay collision.

Public docs:

- `https://github.com/YHK-UEPlugins-Public/018_UEGaussianSplatting_Public`

Important note:

- Verify the currently available marketplace build works in your installed Unreal version before spending time on a production scene.
- The public docs are useful, but they are not the same thing as a guarantee of current UE 5.7 marketplace availability.

### Secondary path if you want the Polycam tutorial exactly

You can also try the `Luma AI` Unreal plugin path, because Polycam's official Unreal article points to it directly.

Why it is viable:

- Luma's Fab listing explicitly says it can import `.ply` files and generate ready-to-use Blueprints.
- It accepts both Luma-native captures and `.ply` scenes exported from other tools.
- This matches the runtime adapter already in this repo, which can spawn plugin-generated actor assets from `ExternalPresentationAsset`.

Important constraint:

- Polycam's official article currently names `Unreal Engine 5.3.2` as the required version for that workflow.
- Our live game project is already on `UE 5.7`.
- That means you should treat the Luma path as a compatibility test, not an assumption.

Official references:

- `https://learn.poly.cam/hc/en-us/articles/42506758152084-Export-Gaussian-Splats-from-Polycam-to-Unreal-Engine`
- `https://www.fab.com/listings/b52460e0-3ace-465e-a378-495a5531e318`

Practical recommendation:

- If you want the lowest-risk path for the main game project, try `UEGaussianSplatting` first.
- If you specifically want the Polycam + Luma workflow, test it in a scratch Unreal project before trying to standardize it for the main game.

### Do not use first on this Mac setup

Avoid `XVERSE XV3dGS` as the first integration on this machine.

Reason:

- The current public plugin metadata lists `SupportedTargetPlatforms: ["Win64"]`.
- It may still be useful later for Windows builds, but it is not the right first plugin for the Mac editor path.

References:

- `https://github.com/xverse-engine/XScene-UEPlugin/blob/main/UEPlugin/README.md`
- `https://github.com/xverse-engine/XScene-UEPlugin/blob/main/UEPlugin/Plugins/XV3dGS/XV3dGS.uplugin`

## Asset acquisition options

### Option 0: Use a public no-login sample first

Fastest way to validate the Unreal import path without depending on a Polycam or Fab account.

Public sample source:

- XVERSE's public demo folder currently exposes `demo_fox_gs.ply` and `demo_office_gs.ply`.
- These are directly downloadable without login and are good enough to validate import, actor spawning, and runtime presentation.
- They are not a complete destination-pack source because they do not include a matching `GLTF` collision mesh.
- NVIDIA's public `vk_gaussian_splatting` sample set also exposes real-place splats like `fountain_place.zip` and `fountain_sindelfingen.zip`.
- Those are larger, but they are better first destination candidates because they are actual places instead of isolated objects.

Repo helper:

- `tools/download-public-gaussian-splat-sample.sh`
- `tools/import-public-gaussian-destination.sh`

Example:

```bash
tools/download-public-gaussian-splat-sample.sh
tools/download-public-gaussian-splat-sample.sh demo-office
tools/import-unreal-gaussian-splat-sample.sh
tools/create-unreal-gaussian-destination-pack.sh
tools/import-public-gaussian-destination.sh fountain-place
```

Verified sample:

- `demo_fox_gs.ply`
- direct public download succeeded on `2026-04-12`
- roughly `48.8 MB`
- `PLY` binary, little endian
- `206346` vertices in the file header

Important note:

- The sample source is not the same thing as the runtime plugin provider.
- If you import `demo_fox_gs.ply` with `NanoGS`, set `GaussianProvider = NanoGS`.
- If you import the same `PLY` with another plugin, set the provider to that plugin instead.

### Option 1: Use an existing Polycam splat

Fastest way to test the game path.

1. Find or save a Gaussian Splat capture in Polycam.
2. Export the splat as `PLY`.
3. Export the geometry proxy as `GLTF` if available.
4. Import the `PLY` with the Unreal splat plugin.
5. Import the `GLTF` as the coarse collision mesh if you want physical terrain immediately.

### Option 2: Capture your own location

Better long-term workflow.

1. Capture with phone, drone, or multi-view photos.
2. Process into a splat in Polycam or another capture tool.
3. Export `PLY`.
4. Export a coarse mesh or derive a collision proxy separately.

## What the game now expects

The Unreal runtime is already ready for one of these two bindings inside a `UParaglideDestinationPack`:

- `ExternalPresentationActorClass`
- `ExternalPresentationAsset`

Use `ExternalPresentationAsset` when the plugin import gives you a Blueprint actor asset.
Use `ExternalPresentationActorClass` when the plugin gives you a directly usable actor class.

Relevant code:

- `ParaglideTheWorldUE/Source/ParaglideTheWorldUE/World/Destinations/ParaglideDestinationPack.h`
- `ParaglideTheWorldUE/Source/ParaglideTheWorldUE/World/Presentation/ParaglideWorldPresentationComponent.cpp`

## First real test flow

1. Install a Mac-capable Unreal Gaussian-splat plugin.
2. Import one `.ply` splat scene.
3. Confirm Unreal created either:
   - an actor Blueprint asset, or
   - an actor class
4. Create a `UParaglideDestinationPack` asset.
5. Set:
   - `DefaultPresentationMode = Hybrid` or `GaussianSplatExternalPlugin`
   - `GaussianProvider` to match the plugin path
   - `ExternalPresentationAsset` or `ExternalPresentationActorClass`
6. Assign that destination pack to `DefaultDestinationPack` on the active game mode.
7. Play in editor and watch the HUD world line.

Expected HUD result:

- destination name visible
- requested mode and runtime mode visible
- Gaussian provider visible
- `live` actor count above `0`
- `loaded` chunk count above `0`

If `live` stays `0`, the asset path is wrong or the imported asset is not a spawnable actor.

## Current repo validation path

For the vendored `NanoGS` path, the repo now has a complete public-sample validation flow:

1. `tools/download-public-gaussian-splat-sample.sh`
2. `tools/import-unreal-gaussian-splat-sample.sh`
3. `tools/create-unreal-gaussian-destination-pack.sh`
4. `tools/import-public-gaussian-destination.sh`

Current validated assets:

- imported Gaussian asset:
  - `/Game/GaussianSplats/Samples/demo_fox_gs.demo_fox_gs`
- generated destination pack:
  - `/Game/Destinations/Samples/PTW_DemoFoxDestinationPack.PTW_DemoFoxDestinationPack`

This specific sample is for pipeline validation only. It is not a playable terrain destination.

Current validated real-location sample:

- imported Gaussian asset:
  - `/Game/GaussianSplats/Public/fountain_place.fountain_place`
- generated destination pack:
  - `/Game/Destinations/Public/PTW_FountainPlaceDestinationPack.PTW_FountainPlaceDestinationPack`
- default game-mode config now points to that destination pack in `ParaglideTheWorldUE/Config/DefaultGame.ini`

Practical result:

- the Unreal project can now ingest a public real-place `.ply`
- wrap it in a destination pack
- and activate it without manual editor changes

## Collision guidance

Do not let the splat asset become the authoritative gameplay surface.

Keep this split:

- splat asset: visual world
- mesh proxy: collision, landing, traces, terrain queries
- airflow metadata: ridge lift, thermals, sink, wind shaping

That is the stable production model.

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

### Recommended first plugin path on Mac

Try `UEGaussianSplatting` first.

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

## Collision guidance

Do not let the splat asset become the authoritative gameplay surface.

Keep this split:

- splat asset: visual world
- mesh proxy: collision, landing, traces, terrain queries
- airflow metadata: ridge lift, thermals, sink, wind shaping

That is the stable production model.

import os

import unreal


def env(name: str, default: str) -> str:
    value = os.environ.get(name, "").strip()
    return value or default


def to_text(value: str):
    return unreal.TextLibrary.conv_string_to_text(value)


source_asset_reference = env(
    "PTW_GS_SOURCE_ASSET",
    "/Game/GaussianSplats/Samples/demo_fox_gs.demo_fox_gs",
)
destination_path = env("PTW_DEST_PACK_PATH", "/Game/Destinations/Samples")
destination_name = env("PTW_DEST_PACK_NAME", "PTW_DemoFoxDestinationPack")
destination_id = env("PTW_DEST_PACK_ID", "demo_fox")
display_name = env("PTW_DEST_PACK_DISPLAY_NAME", "Demo Fox Gaussian Sample")
summary = env(
    "PTW_DEST_PACK_SUMMARY",
    "Public Gaussian splat pipeline validation sample wired into the Unreal destination-pack system.",
)

destination_asset_reference = f"{destination_path}/{destination_name}.{destination_name}"
destination_asset_path = f"{destination_path}/{destination_name}"

source_asset = unreal.EditorAssetLibrary.load_asset(source_asset_reference)
if source_asset is None:
    raise RuntimeError(f"Gaussian source asset could not be loaded: {source_asset_reference}")

destination_pack = None
if unreal.EditorAssetLibrary.does_asset_exist(destination_asset_path):
    destination_pack = unreal.EditorAssetLibrary.load_asset(destination_asset_reference)
if destination_pack is None:
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.ParaglideDestinationPack)
    destination_pack = asset_tools.create_asset(
        destination_name,
        destination_path,
        unreal.ParaglideDestinationPack,
        factory,
    )

if destination_pack is None:
    raise RuntimeError(f"Failed to create or load destination pack asset: {destination_asset_reference}")

presentation_mode = unreal.ParaglideDestinationPresentationMode.GAUSSIAN_SPLAT_EXTERNAL_PLUGIN
provider = unreal.ParaglideGaussianPresentationProvider.NANO_GS

chunk = unreal.ParaglideDestinationChunk()
chunk.set_editor_property("chunk_id", "demo_fox_chunk")
chunk.set_editor_property("label", to_text("Demo Fox Gaussian Sample"))
chunk.set_editor_property("presentation_mode", presentation_mode)
chunk.set_editor_property("local_transform", unreal.Transform())
chunk.set_editor_property("bounds_extent_meters", unreal.Vector(5.0, 5.0, 5.0))
chunk.set_editor_property("external_presentation_asset", unreal.SoftObjectPath(source_asset_reference))
chunk.set_editor_property("gaussian_provider", provider)

layer = unreal.ParaglideDestinationPresentationLayer()
layer.set_editor_property("layer_id", "gaussian_sample")
layer.set_editor_property("label", to_text("Gaussian Sample Layer"))
layer.set_editor_property("presentation_mode", presentation_mode)
layer.set_editor_property("gaussian_provider", provider)
layer.set_editor_property("chunks", [chunk])

destination_pack.set_editor_property("destination_id", destination_id)
destination_pack.set_editor_property("display_name", to_text(display_name))
destination_pack.set_editor_property("summary", to_text(summary))
destination_pack.set_editor_property("default_presentation_mode", presentation_mode)
destination_pack.set_editor_property("presentation_layers", [layer])

if not unreal.EditorAssetLibrary.save_loaded_asset(destination_pack):
    raise RuntimeError(f"Failed to save destination pack asset: {destination_asset_reference}")

unreal.log(f"Destination pack saved: {destination_asset_reference}")
unreal.log(f"Bound Gaussian source asset: {source_asset_reference}")

import os
from pathlib import Path

import unreal


def env(name: str, default: str) -> str:
    value = os.environ.get(name, "").strip()
    return value or default


def env_bool(name: str, default: bool = False) -> bool:
    value = os.environ.get(name, "").strip().lower()
    if not value:
        return default
    return value in {"1", "true", "yes", "on"}


def to_text(value: str):
    return unreal.TextLibrary.conv_string_to_text(value)


def parse_csv_floats(value: str, fallback):
    raw = [part.strip() for part in value.split(",") if part.strip()]
    if len(raw) != len(fallback):
        return fallback
    try:
        return tuple(float(part) for part in raw)
    except ValueError:
        return fallback


def resolve_presentation_mode(name: str):
    mapping = {
        "PROTOTYPE_PROCEDURAL": unreal.ParaglideDestinationPresentationMode.PROTOTYPE_PROCEDURAL,
        "GAUSSIAN_SPLAT_EXTERNAL_PLUGIN": unreal.ParaglideDestinationPresentationMode.GAUSSIAN_SPLAT_EXTERNAL_PLUGIN,
        "HYBRID": unreal.ParaglideDestinationPresentationMode.HYBRID,
    }
    return mapping.get(name.upper(), unreal.ParaglideDestinationPresentationMode.GAUSSIAN_SPLAT_EXTERNAL_PLUGIN)


def resolve_provider(name: str):
    mapping = {
        "NONE": unreal.ParaglideGaussianPresentationProvider.NONE,
        "NANO_GS": unreal.ParaglideGaussianPresentationProvider.NANO_GS,
        "LUMA_AI": unreal.ParaglideGaussianPresentationProvider.LUMA_AI,
        "XVERSE_XV3D_GS": unreal.ParaglideGaussianPresentationProvider.XVERSE_XV3D_GS,
        "UE_GAUSSIAN_SPLATTING": unreal.ParaglideGaussianPresentationProvider.UE_GAUSSIAN_SPLATTING,
    }
    return mapping.get(name.upper(), unreal.ParaglideGaussianPresentationProvider.NANO_GS)


def upsert_ini_values(path: Path, section: str, values: dict[str, str]) -> None:
    lines = path.read_text().splitlines() if path.exists() else []

    start_index = None
    end_index = len(lines)
    for index, line in enumerate(lines):
        stripped = line.strip()
        if stripped == section:
            start_index = index
            continue
        if start_index is not None and stripped.startswith("[") and stripped.endswith("]"):
            end_index = index
            break

    if start_index is None:
        if lines and lines[-1].strip():
            lines.append("")
        lines.append(section)
        for key, value in values.items():
            lines.append(f"{key}={value}")
    else:
        section_lines = lines[start_index + 1 : end_index]
        filtered_lines = []
        for line in section_lines:
            stripped = line.strip()
            if any(stripped.startswith(f"{key}=") for key in values):
                continue
            filtered_lines.append(line)

        replacement = [section]
        replacement.extend(filtered_lines)
        replacement.extend(f"{key}={value}" for key, value in values.items())
        lines[start_index:end_index] = replacement

    path.write_text("\n".join(lines).rstrip() + "\n")


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
layer_id = env("PTW_DEST_LAYER_ID", f"{destination_id}_layer")
layer_label = env("PTW_DEST_LAYER_LABEL", f"{display_name} Layer")
chunk_id = env("PTW_DEST_CHUNK_ID", f"{destination_id}_chunk")
chunk_label = env("PTW_DEST_CHUNK_LABEL", display_name)
presentation_mode_name = env("PTW_DEST_PRESENTATION_MODE", "GAUSSIAN_SPLAT_EXTERNAL_PLUGIN")
provider_name = env("PTW_DEST_GAUSSIAN_PROVIDER", "NANO_GS")
runtime_mode_name = env("PTW_DEFAULT_RUNTIME_MODE", "Hybrid")
set_as_default = env_bool("PTW_SET_AS_DEFAULT", False)

chunk_extent_xyz = parse_csv_floats(env("PTW_DEST_CHUNK_EXTENT_METERS", "5,5,5"), (5.0, 5.0, 5.0))
chunk_translation_xyz = parse_csv_floats(env("PTW_DEST_CHUNK_TRANSLATION_METERS", "0,0,0"), (0.0, 0.0, 0.0))
chunk_rotation_rpy = parse_csv_floats(env("PTW_DEST_CHUNK_ROTATION_DEG", "0,0,0"), (0.0, 0.0, 0.0))
chunk_scale_xyz = parse_csv_floats(env("PTW_DEST_CHUNK_SCALE", "1,1,1"), (1.0, 1.0, 1.0))

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

presentation_mode = resolve_presentation_mode(presentation_mode_name)
provider = resolve_provider(provider_name)

chunk_transform = unreal.Transform()
if (
    chunk_translation_xyz != (0.0, 0.0, 0.0)
    or chunk_rotation_rpy != (0.0, 0.0, 0.0)
    or chunk_scale_xyz != (1.0, 1.0, 1.0)
):
    unreal.log_warning("Non-default chunk transforms are not yet applied by the automation path; using identity transform.")

chunk = unreal.ParaglideDestinationChunk()
chunk.set_editor_property("chunk_id", chunk_id)
chunk.set_editor_property("label", to_text(chunk_label))
chunk.set_editor_property("presentation_mode", presentation_mode)
chunk.set_editor_property("local_transform", chunk_transform)
chunk.set_editor_property("bounds_extent_meters", unreal.Vector(*chunk_extent_xyz))
chunk.set_editor_property("external_presentation_asset", unreal.SoftObjectPath(source_asset_reference))
chunk.set_editor_property("gaussian_provider", provider)

layer = unreal.ParaglideDestinationPresentationLayer()
layer.set_editor_property("layer_id", layer_id)
layer.set_editor_property("label", to_text(layer_label))
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

if set_as_default:
    config_dir = Path(unreal.Paths.project_config_dir())
    default_game_ini = config_dir / "DefaultGame.ini"
    upsert_ini_values(
        default_game_ini,
        "[/Script/ParaglideTheWorldUE.ParaglideGameMode]",
        {
            "DefaultDestinationPack": destination_asset_reference,
            "PreferredPresentationMode": runtime_mode_name,
        },
    )
    unreal.log(f"Updated default destination config: {default_game_ini}")

unreal.log(f"Destination pack saved: {destination_asset_reference}")
unreal.log(f"Bound Gaussian source asset: {source_asset_reference}")

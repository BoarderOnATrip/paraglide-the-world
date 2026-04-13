import os
from pathlib import Path

import unreal


def env(name: str, default: str) -> str:
    value = os.environ.get(name, "").strip()
    return value or default


input_path = Path(env("PTW_GS_INPUT", "/tmp/paraglide-the-world-gaussian-splat-samples/demo-fox/demo_fox_gs.ply"))
destination_path = env("PTW_GS_DEST_PATH", "/Game/GaussianSplats/Samples")
destination_name = env("PTW_GS_DEST_NAME", input_path.stem)

if not input_path.exists():
    raise RuntimeError(f"Gaussian splat input file not found: {input_path}")

unreal.log(f"Importing Gaussian splat from {input_path}")
unreal.log(f"Destination path: {destination_path}")
unreal.log(f"Destination name: {destination_name}")

task = unreal.AssetImportTask()
task.filename = str(input_path)
task.destination_path = destination_path
task.destination_name = destination_name
task.automated = True
task.replace_existing = True
task.replace_existing_settings = True
task.save = True

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks([task])

imported_paths = list(task.imported_object_paths)
if not imported_paths:
    raise RuntimeError("Import completed without any imported object paths.")

for imported_path in imported_paths:
    asset = unreal.EditorAssetLibrary.load_asset(imported_path)
    if asset is None:
        raise RuntimeError(f"Imported asset could not be loaded: {imported_path}")
    unreal.log(f"Imported asset: {imported_path} ({asset.get_class().get_name()})")
    unreal.EditorAssetLibrary.save_loaded_asset(asset)

unreal.log("Gaussian splat import completed successfully.")

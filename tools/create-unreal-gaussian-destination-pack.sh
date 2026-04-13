#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project_path="${repo_root}/ParaglideTheWorldUE/ParaglideTheWorldUE.uproject"
script_path="${repo_root}/ParaglideTheWorldUE/Scripts/create_gaussian_destination_pack.py"
editor_cmd="/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor-Cmd"

source_asset_reference="${1:-/Game/GaussianSplats/Samples/demo_fox_gs.demo_fox_gs}"
destination_path="${2:-/Game/Destinations/Samples}"
destination_name="${3:-PTW_DemoFoxDestinationPack}"
set_as_default="${4:-${PTW_SET_AS_DEFAULT:-0}}"

if [[ ! -x "$editor_cmd" ]]; then
  echo "UnrealEditor-Cmd not found: $editor_cmd" >&2
  exit 1
fi

export PTW_GS_SOURCE_ASSET="$source_asset_reference"
export PTW_DEST_PACK_PATH="$destination_path"
export PTW_DEST_PACK_NAME="$destination_name"
export PTW_SET_AS_DEFAULT="$set_as_default"

echo "Creating destination pack ${PTW_DEST_PACK_PATH}/${PTW_DEST_PACK_NAME} from ${PTW_GS_SOURCE_ASSET}"
"$editor_cmd" "$project_path" \
  -run=PythonScript \
  -Script="$script_path" \
  -nop4 \
  -nosplash \
  -nullrhi \
  -unattended \
  -stdout \
  -FullStdOutLogOutput

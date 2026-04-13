#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project_path="${repo_root}/ParaglideTheWorldUE/ParaglideTheWorldUE.uproject"
script_path="${repo_root}/ParaglideTheWorldUE/Scripts/import_nanogs_sample.py"
editor_cmd="/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor-Cmd"

input_path="${1:-/tmp/paraglide-the-world-gaussian-splat-samples/demo-fox/demo_fox_gs.ply}"
destination_path="${2:-/Game/GaussianSplats/Samples}"
destination_name="${3:-}"

if [[ ! -f "$input_path" ]]; then
  "${repo_root}/tools/download-public-gaussian-splat-sample.sh" demo-fox /tmp/paraglide-the-world-gaussian-splat-samples
fi

if [[ ! -f "$input_path" ]]; then
  echo "Input PLY not found: $input_path" >&2
  exit 1
fi

if [[ ! -x "$editor_cmd" ]]; then
  echo "UnrealEditor-Cmd not found: $editor_cmd" >&2
  exit 1
fi

if [[ -z "$destination_name" ]]; then
  destination_name="$(basename "$input_path" .ply)"
fi

export PTW_GS_INPUT="$input_path"
export PTW_GS_DEST_PATH="$destination_path"
export PTW_GS_DEST_NAME="$destination_name"

echo "Importing ${PTW_GS_INPUT} into ${PTW_GS_DEST_PATH}/${PTW_GS_DEST_NAME}"
"$editor_cmd" "$project_path" \
  -run=PythonScript \
  -Script="$script_path" \
  -nop4 \
  -nosplash \
  -nullrhi \
  -unattended \
  -stdout \
  -FullStdOutLogOutput

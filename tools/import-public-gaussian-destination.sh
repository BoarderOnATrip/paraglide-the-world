#!/usr/bin/env bash

set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  tools/import-public-gaussian-destination.sh [sample-alias] [set-as-default]

Examples:
  tools/import-public-gaussian-destination.sh fountain-place
  tools/import-public-gaussian-destination.sh fountain-place 0

Notes:
  - Downloads a public PLY sample if needed.
  - Imports it into Unreal through NanoGS.
  - Creates a destination-pack asset and optionally sets it as the active default world.
EOF
}

if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
  usage
  exit 0
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sample="${1:-fountain-place}"
set_as_default="${2:-1}"
sample_root="/tmp/paraglide-the-world-gaussian-splat-samples"

"${repo_root}/tools/download-public-gaussian-splat-sample.sh" "$sample" "$sample_root"

case "$sample" in
  demo-fox) input_path="${sample_root}/demo-fox/demo_fox_gs.ply" ;;
  demo-office) input_path="${sample_root}/demo-office/demo_office_gs.ply" ;;
  fountain-place) input_path="${sample_root}/fountain-place/fountain_place.ply" ;;
  fountain-sindelfingen) input_path="${sample_root}/fountain-sindelfingen/fountain_sindelfingen.ply" ;;
  *)
    echo "Unknown sample alias: $sample" >&2
    usage >&2
    exit 1
    ;;
esac

asset_name="$(basename "$input_path" .ply)"
dest_id="${asset_name}"
dest_pack_name="PTW_$(python3 - <<'PY' "$asset_name"
import sys
parts = sys.argv[1].replace('-', '_').split('_')
print(''.join(part[:1].upper() + part[1:] for part in parts if part))
PY
)DestinationPack"
display_name="$(python3 - <<'PY' "$asset_name"
import sys
parts = sys.argv[1].replace('-', '_').split('_')
print(' '.join(part[:1].upper() + part[1:] for part in parts if part))
PY
)"

case "$sample" in
  fountain-place)
    summary="Public NVIDIA Gaussian splat of Fountain Place wired into the Unreal destination system as the first real-location sample."
    chunk_extent="500,565,560"
    ;;
  fountain-sindelfingen)
    summary="Public NVIDIA Gaussian splat of Fountain Sindelfingen wired into the Unreal destination system as a real-location sample."
    chunk_extent="500,500,500"
    ;;
  *)
    summary="Public Gaussian splat sample wired into the Unreal destination system."
    chunk_extent="50,50,50"
    ;;
esac

"${repo_root}/tools/import-unreal-gaussian-splat-sample.sh" "$input_path" "/Game/GaussianSplats/Public" "$asset_name"

export PTW_DEST_PACK_ID="$dest_id"
export PTW_DEST_PACK_DISPLAY_NAME="$display_name"
export PTW_DEST_PACK_SUMMARY="$summary"
export PTW_DEST_LAYER_ID="${dest_id}_layer"
export PTW_DEST_LAYER_LABEL="${display_name} Layer"
export PTW_DEST_CHUNK_ID="${dest_id}_chunk"
export PTW_DEST_CHUNK_LABEL="$display_name"
export PTW_DEST_CHUNK_EXTENT_METERS="$chunk_extent"
export PTW_DEST_GAUSSIAN_PROVIDER="NANO_GS"
export PTW_DEST_PRESENTATION_MODE="GAUSSIAN_SPLAT_EXTERNAL_PLUGIN"
export PTW_DEFAULT_RUNTIME_MODE="Hybrid"

"${repo_root}/tools/create-unreal-gaussian-destination-pack.sh" \
  "/Game/GaussianSplats/Public/${asset_name}.${asset_name}" \
  "/Game/Destinations/Public" \
  "$dest_pack_name" \
  "$set_as_default"

echo
echo "Imported sample destination:"
echo "  Gaussian asset: /Game/GaussianSplats/Public/${asset_name}.${asset_name}"
echo "  Destination pack: /Game/Destinations/Public/${dest_pack_name}.${dest_pack_name}"
echo "  Set as default: ${set_as_default}"

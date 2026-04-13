#!/usr/bin/env bash

set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  tools/download-public-gaussian-splat-sample.sh [demo-fox|demo-office|fountain-place|fountain-sindelfingen] [output-root]

Examples:
  tools/download-public-gaussian-splat-sample.sh
  tools/download-public-gaussian-splat-sample.sh demo-office
  tools/download-public-gaussian-splat-sample.sh demo-fox /tmp/paraglide-splats

Notes:
  - Downloads a public sample PLY with no account login required.
  - Default output root: /tmp/paraglide-the-world-gaussian-splat-samples
  - These samples are useful for render-pipeline validation, not final collision.
EOF
}

if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
  usage
  exit 0
fi

sample="${1:-demo-fox}"
output_root="${2:-/tmp/paraglide-the-world-gaussian-splat-samples}"

case "$sample" in
  demo-fox)
    filename="demo_fox_gs.ply"
    download_url="https://drive.google.com/uc?export=download&id=1DcBErOxImQXtvIUksro4MfxZdKtAEA_b"
    ;;
  demo-office)
    filename="demo_office_gs.ply"
    download_url="https://drive.google.com/uc?export=download&id=162AL3-nFH91BSnM6idFMgANo5AvYbRd2"
    ;;
  fountain-place)
    filename="fountain_place.ply"
    download_url="https://developer.download.nvidia.com/ProGraphics/nvpro-samples/fountain_place.zip"
    ;;
  fountain-sindelfingen)
    filename="fountain_sindelfingen.ply"
    download_url="https://developer.download.nvidia.com/ProGraphics/nvpro-samples/fountain_sindelfingen.zip"
    ;;
  *)
    echo "Unknown sample: $sample" >&2
    usage >&2
    exit 1
    ;;
esac

destination_dir="${output_root%/}/${sample}"
destination_file="${destination_dir}/${filename}"

mkdir -p "$destination_dir"

echo "Downloading ${sample} to ${destination_file}"
case "$sample" in
  fountain-place|fountain-sindelfingen)
    archive_path="${destination_dir}/${sample}.zip"
    extract_root="${destination_dir}/_extract"
    rm -rf "$extract_root"
    curl --fail --location --output "$archive_path" "$download_url"
    unzip -q -o "$archive_path" -d "$extract_root"
    extracted_file="$(find "$extract_root" -type f -name '*.ply' | head -n 1)"
    if [[ -z "$extracted_file" ]]; then
      echo "No PLY found in ${archive_path}" >&2
      exit 1
    fi
    cp "$extracted_file" "$destination_file"
    ;;
  *)
    curl --fail --location --output "$destination_file" "$download_url"
    ;;
esac

echo
echo "Saved sample:"
ls -lh "$destination_file"
file "$destination_file"

cat <<EOF

Next steps:
1. Import ${filename} into Unreal with a Gaussian-splat plugin.
2. If the plugin generates a Blueprint actor asset, bind that asset in UParaglideDestinationPack.ExternalPresentationAsset.
3. Set GaussianProvider to the plugin that created the Unreal actor, not the original sample source.
EOF

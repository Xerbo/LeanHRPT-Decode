#!/usr/bin/env sh
# https://github.com/Xerbo/LeanHRPT-Decode/wiki/Projection
temp="$(mktemp)"

gdal_translate $(cat "$2") "$1" -of GTiff "$temp"
gdalwarp -overwrite -r bilinear -tps -t_srs "epsg:4326" "$temp" "$3"

rm "$temp"

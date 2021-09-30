# https://github.com/Xerbo/LeanHRPT-Decode/wiki/Projection
$gcps = $($(Get-Content $args[1]) -split " ")
$temp = New-TemporaryFile

gdal_translate $gcps $args[0] -of GTiff $temp
gdalwarp -r bilinear -tps -t_srs "epsg:4326" $temp $args[2]

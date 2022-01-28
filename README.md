# LeanHRPT Decode

[![Build](https://github.com/Xerbo/LeanHRPT-Decode/actions/workflows/build.yml/badge.svg)](https://github.com/Xerbo/LeanHRPT-Decode/actions/workflows/build.yml)

LeanHRPT is an easy to use and powerful tool for the manipulation of Level 0 HRPT data. Also see [LeanHRPT Demod](https://github.com/Xerbo/LeanHRPT-Demod).

![Screenshot of LeanHRPT's GUI](images/gui.webp)

## Usage

See the [wiki](https://github.com/Xerbo/LeanHRPT-Decode/wiki).

## FAQ

### Map overlays don't work

See [the wiki](https://github.com/Xerbo/LeanHRPT-Decode/wiki/Geomanipulation#installing-gdal)

## Installation

Compiled builds for Linux and Windows are available from the [releases](https://github.com/Xerbo/LeanHRPT-Decode/releases) page.

## Building from source

For those who wish to compile from source you will need `qtbase5-dev` (at least 5.14), [`muparser`](https://github.com/beltoforion/muparser) and [`libpredict`](https://github.com/la1k/libpredict) installed. After that you can compile with:

```sh
mkdir build && cd build
cmake ..
make
```

You may also wish to symlink the configuration files into the build directory:

```sh
# Assuming you are in LeanHRPT-Decode/build
ln -s ../presets.ini
ln -s ../calibration.ini
ln -s ../projection.ini
ln -s ../gradients.ini
ln -s ../logo128.png
```

And then run the resulting binary. Alternatively, to install system wide:

```
sudo make install
```

## Example results

|![](images/fy.webp)FengYun-3B with the "NCSWIR" preset|![](images/metop.webp)MetOp-C 321 with a map overlay|
 -|-
|![](images/ir.webp)**Projected NOAA IR image**|![](images/mhs.png)<br/>**MHS 112 Composite with "brightness only" equalization**|


## Input file format

Input files should be:

 - NOAA - raw data, raw16 or HRP
 - Meteor - raw data
 - MetOp - CADUs or VCDUs
 - FengYun - CADUs or VCDUs

## Credits

 - Geotranslation code - taken from `MeteorDemod`

# LeanHRPT Decode

[![Build](https://github.com/Xerbo/LeanHRPT-Decode/actions/workflows/build.yml/badge.svg)](https://github.com/Xerbo/LeanHRPT-Decode/actions/workflows/build.yml)

A high quality, easy to use HRPT decoder.

## Usage

See the [wiki](https://github.com/Xerbo/LeanHRPT-Decode/wiki).

## Installation

Compiled builds for Linux and Windows are available from the [releases](https://github.com/Xerbo/LeanHRPT-Decode/releases) page.

## Building from source

For those who wish to compile from source you will need `qtbase5-dev` (at least 5.14), [`libcorrect`](https://github.com/quiet/libcorrect), [`muparser`](https://github.com/beltoforion/muparser) and [`libpredict`](https://github.com/la1k/libpredict) installed. After that you can compile with:

```sh
mkdir build && cd build
cmake ..
make
```

You may also wish to symlink the configuration files into the build directory:

```sh
# Assuming you are in LeanHRPT-Decode/build
ln -s ../presets.ini ..
ln -s ../calibration.ini ..
ln -s ../projection.ini ..
```

And then run the resulting binary.

## Example results

|![](images/fy.webp)FengYun-3B with the "NCSWIR" preset|![](images/metop.webp)MetOp-C 321 with a map overlay|
 -|-
|![](images/ir.webp)**Projected NOAA IR image**|![](images/mhs.png)<br/>**MHS 112 Composite with "brightness only" equalization**|


## Input file format

Input files should be:

 - NOAA - raw data, bi-phase/manchester decoded or raw16
 - Meteor - raw data, bi-phase/manchester decoded or CADUs (deframed)
 - MetOp - output from viterbi, CADUs (deframed, derandomized with RS applied) or VCDUs
 - FengYun - output from viterbi, CADUs (deframed, derandomized with RS applied) or VCDUs

## Credits

 - Reed Solomon - adapted from `libsathelper`
 - Geotranslation code - taken from `MeteorDemod`

# LeanHRPT Decode

<img alt="Screenshot of LeanHRPT's GUI" src="images/gui.webp" align="right" width="40%">

[![Build](https://github.com/Xerbo/LeanHRPT-Decode/actions/workflows/build.yml/badge.svg)](https://github.com/Xerbo/LeanHRPT-Decode/actions/workflows/build.yml)

LeanHRPT is an easy to use and powerful tool for the manipulation of Level 0 HRPT data. Also see [LeanHRPT Demod](https://github.com/Xerbo/LeanHRPT-Demod).

## Installation

Compiled builds for Linux and Windows are available from the [releases](https://github.com/Xerbo/LeanHRPT-Decode/releases) page.

## Usage

See the [wiki](https://github.com/Xerbo/LeanHRPT-Decode/wiki).

## Example results

|FengYun-3B with the NCSWIR preset|MetOp-C SWIR with a map overlay|Projected NOAA IR image|
|-|-|-|
|![](images/fy.webp)|![](images/metop.webp)|![](images/ir.webp)|

## Advanced

### Building from source

You will need Qt (at least 5.14), [`muparser`](https://github.com/beltoforion/muparser), [`libpredict`](https://github.com/la1k/libpredict) and [`shapelib`](https://github.com/OSGeo/shapelib) installed.

While MacOS isn't officially supported, compiling should be pretty similar to the steps below.

#### Dependencies Installation

```sh
# Debian/Ubuntu
sudo apt install cmake gcc g++ qtbase5-dev libmuparser-dev libshp-dev
git clone -b v2.0.0 https://github.com/la1k/libpredict && cd libpredict
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
sudo cmake --install .
```

```sh
# MSYS2
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-qt5-base mingw-w64-x86_64-muparser mingw-w64-x86_64-shapelib mingw-w64-x86_64-ninja git
git clone -b v2.0.0 https://github.com/la1k/libpredict && cd libpredict
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/mingw64 -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cmake --install .
```

#### Development Setup

```sh
mkdir build && cd build
cmake ..
cmake --build .
ln -s ../presets.ini
ln -s ../calibration.ini
ln -s ../projection.ini
ln -s ../gradients.ini
ln -s ../logo128.png
```

### Input file format

Input files should be:

 - NOAA - raw data, raw16 or HRP
 - Meteor - raw data
 - MetOp - CADUs or VCDUs
 - FengYun - CADUs or VCDUs

## Credits

 - [`MeteorDemod`](https://github.com/Digitelektro/MeteorDemod) - geotransformation code
 - [JM (@g5juergenm)](https://twitter.com/g5juergenm) - testing and feedback
 - KentuckyFriedData - donation

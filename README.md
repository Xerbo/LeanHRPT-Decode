# LeanHRPT Decode

[![Build](https://github.com/Xerbo/LeanHRPT-Decode/actions/workflows/build.yml/badge.svg)](https://github.com/Xerbo/LeanHRPT-Decode/actions/workflows/build.yml)

A high quality, easy to use HRPT decoder.

## Usage

See the [wiki](https://github.com/Xerbo/LeanHRPT-Decode/wiki).

## Installation

Compiled builds for Linux and Windows are available from the [releases](https://github.com/Xerbo/LeanHRPT-Decode/releases) page.

## Building from source

For those who wish to compile from source you will need `qtbase5-dev`, [`libcorrect`](https://github.com/quiet/libcorrect), [`muparser`](https://github.com/beltoforion/muparser) and [`libpredict`](https://github.com/la1k/libpredict) installed. After that you can compile with:

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

### Packaging your own builds

TODO

## Example results

|Satellite|LeanHRPT|SatDump|
 -|-|-
|MetOp<br/>(Airspy Mini and SAWBird GOES+, received by Byggemandboesen)|![A MetOp image from LeanHRPT](images/metop-example.jpg)|![A MetOp image from SatDump](images/metop-satdump.jpg)|
|FengYun<br/>(Airspy Mini and SAWBird GOES+, received by Byggemandboesen)|![A FengYun image from LeanHRPT](images/fengyun-example.jpg)|![A FengYun image from SatDump](images/fengyun-satdump.jpg)|
|Meteor<br/>(NooElec NESDR SMArt @ 2.4msps and SAWBird GOES+, received by me)|![A Meteor image from LeanHRPT](images/meteor-example.jpg)|![A Meteor image from SatDump](images/meteor-satdump.jpg)|
|NOAA<br/>(NooElec NESDR SMArt @ 2.4msps and SAWBird GOES+, received by me)|![A NOAA image from LeanHRPT](images/noaa-example.jpg)|![A NOAA image from SatDump](images/noaa-satdump.jpg)|

## Input file format

Input files should be:

 - NOAA - raw data, bi-phase/manchester/PM decoded and not raw16
 - Meteor - raw data, bi-phase/manchester/PM decoded *or* CADUs (deframed)
 - MetOp - output from viterbi *or* CADUs (deframed, derandomized with RS applied)
 - FengYun - output from viterbi *or* CADUs (deframed, derandomized with RS applied)

## Credits

 - Reed Solomon - adapted from `libsathelper`
 - "reckon" - adapted from `noaa-apt`

## Flowcharts

Included in this repository are GNURadio flowcharts for demodulating NOAA, Meteor, FengYun and MetOp. These flowcharts have been created from the ground up using only standard GNURadio blocks, not requiring any OOP modules such as `gr-hrpt`.

All flowcharts have file sources for both 32-bit float (gqrx) and WAV, by default WAV is selected (requires GNURadio >3.9 in order to read RF64 files).

The `NOAA Meteor HRPT Demodulator` flowchart contains a proper PM/bi-phase demodulator which gives a +3db signal increase.

 - For NOAA, use the `NOAA Meteor HRPT Demodulator` flowchart, it **does not** output a framed raw16 file and is incompatible with other software as of now.
 - For Meteor, use the `NOAA Meteor HRPT Demodulator` flowchart, it outputs manchester decoded data, to use the resulting file in MetFy3x simply uncheck "Manchester Decode".
 - For FengYun, use the `Fengyun Demodulator` flowchart, it outputs 8 bit (little endian) signed soft symbols.
 - For MetOp, use the `MetOp Demodulator` flowchart, it outputs 8 bit (little endian) signed soft symbols.

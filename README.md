# LeanHRPT Decode

An easy to use HRPT decoder.																	

## Usage

See the [wiki](https://github.com/Xerbo/LeanHRPT-Decode/wiki).

## Installation

Compiled builds for Linux (and soon Windows) are available from the [releases](https://github.com/Xerbo/LeanHRPT-Decode/releases) page.

For those who wish to compile from source you will need `qtbase5-dev` and [`libcorrect`](https://github.com/quiet/libcorrect) installed. After that you can compile with:

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 4
./LeanHRPT-Decode
```

## Example results

## MetOp

![A MetOp image from LeanHRPT](images/metop-example.jpg)	

## NOAA

![A NOAA image from LeanHRPT](images/noaa-example.jpg)

## FengYun

TODO

## Meteor

TODO

## Input file format	

Input files should be:

 - NOAA - raw data, manchester/PM decoded and not raw16
 - Meteor - raw data, manchester/PM decoded, not run through MetFy3x or similar
 - MetOp - output from viterbi
 - FengYun - output from viterbi/diff

All deframing is done in LeanHRPT-Decode itself.

## Credits

 - Reed Solomon - from `libsathelper`

## Flowcharts

Included in this repository are GNURadio flow graphs for demodulating NOAA, Meteor and Fengyun. These flow graphs have been created from the ground up using only the standard GNURadio blocks, not requiring any OOP modules like gr-hrpt.

All flowgraphs have file sources for both 32 bit float (gqrx) and 16 bit short (SDR#), by default 16 bit short is enabled.

The NOAA/Meteor flowgraph contain a proper PM/Bi-phase/manchester decoder which gives an effective +3db signal increase as well as better tuned clock recovery, giving it an advantage over other, older flowgraphs.

 - For NOAA use the `NOAA Meteor HRPT Demodulator` flow graph, it **does not** output a framed raw16 file and is incompatible with other software as of now
 - For Meteor use the `NOAA Meteor HRPT Demodulator` flow graph, it outputs manchester decoded data, to use the resulting file in other software such as MetFy3x simply uncheck "Manchester Decode"
 - For FengYun use the `Fengyun Demodulator` flow graph, it outputs 8 bit soft symbols
 - For MetOp use the `MetOp Demodulator` flow graph, it outputs 8 bit soft symbols

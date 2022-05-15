#!/bin/bash
cd /root
git clone https://github.com/la1k/libpredict.git && cd libpredict
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j4
make install

cd /root/LeanHRPT-Decode
mkdir build && cd build
git config --global --add safe.directory /root/LeanHRPT-Decode
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j4
make package

#!/bin/bash

mkdir cmake-build
mv get_stocks_data.sh cmake-build
cd cmake-build
cmake ..
make ./linuxEx2
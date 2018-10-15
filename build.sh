#!/bin/bash

# sudo apt install libssl-dev
mkdir build
cd build
cmake .. -DBUILD_TESTING=OFF

#!/bin/bash
if [ -f "./lib/libgtest_main.a" ]; then
    echo "Skipped building gtest: already present."
    exit 0
fi
git clone https://github.com/google/googletest.git -b release-1.11.0
cd googletest
mkdir build
cd build
cmake ..
make
cd ../..
cp -R ./googletest/build/lib ./
cp -R ./googletest/googletest/include/gtest ./include/
cp -R ./googletest/googlemock/include/gmock/ ./include/
rm -rf ./googletest
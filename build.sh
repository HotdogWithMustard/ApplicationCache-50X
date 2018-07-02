#!/bin/bash
BUILD_NAME=ApplicationCache
set -e
pushd tool
make
popd
pushd $BUILD_NAME
xxd -i ApplicationCache.db > ./include/cache.h
make
popd
mkdir -p bin
cp $BUILD_NAME/$BUILD_NAME.bin bin/$BUILD_NAME.bin
tool/bin2js bin/$BUILD_NAME.bin > bin/payload.js
echo "s/###/$(cat bin/payload.js)/" > sed.dat
sed -f "sed.dat" exploit.template > bin/Index.html
rm -f bin/payload.js
rm -f sed.dat
rm -f $BUILD_NAME/$BUILD_NAME.bin

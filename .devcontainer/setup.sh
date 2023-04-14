#!/usr/bin/env bash

sdk_file='qrypt-security-ubuntu.tgz'
curl -s https://qrypt.azureedge.net/sdk/cpp/v0.9.2/qrypt-security-0.9.2-ubuntu.tgz --output $sdk_file
tar -zxvf $sdk_file --strip-components=1 -C QryptSecurity
rm -rf $sdk_file

cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
cmake --build build --config Release
./qrypt test
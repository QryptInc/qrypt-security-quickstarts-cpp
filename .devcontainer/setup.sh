#!/usr/bin/env bash

cd KeyGenDistributed

sdk_file='qrypt-security-ubuntu.tgz'
curl -s https://qrypt.azureedge.net/sdk/cpp/v0.8.6/qrypt-security-0.8.6-ubuntu.tgz --output $sdk_file
tar -zxvf $sdk_file --strip-components=1 -C lib/QryptSecurity
rm -rf $sdk_file

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
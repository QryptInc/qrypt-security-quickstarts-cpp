#!/usr/bin/env bash

set -e

sdk_file='qrypt-security-ubuntu.tgz'
curl -s https://qryptogateway.qrypt.com/sdk/cpp/v0.11.23/qrypt-security-0.11.23-ubuntu.tgz --output $sdk_file
tar -zxvf $sdk_file --strip-components=1 -C QryptSecurity
rm -rf $sdk_file

rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
cmake --build build --config Release

if [[ -n $CODESPACE_NAME ]]; then
    # Running in GitHub Codespaces

    # Host the flask server on port 5000
    gh codespace ports visibility 5000:public -c $CODESPACE_NAME
    nohup python3 /workspaces/qrypt-security-quickstarts-cpp/scripts/flask_app.py >nohup.out 2>&1 &

    echo -e "\nDoes Alice want to send you a metadata file? Provide her with this string to use as the destination:\n$CODESPACE_NAME\n"
fi
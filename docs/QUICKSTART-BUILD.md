## Building this quickstart manually
The QryptSecurity SDK is intended to be run on an Ubuntu 20.04 system with an arm64 architecture, either natively or using an emulated platform. The following commands assume a system configured with OpenSSL, CMake, and g++.

1. [Create a Qrypt account for free](https://portal.qrypt.com/register).
1. On the Qrypt portal, download the Qrypt SDK from "Products > Qrypt SDK" and save the .tgz to the project root.
1. (Optional) On the Qrypt portal, register a personal access token for keygen.
    - Export generated access token as an enviornment variable: `export QRYPT_TOKEN="<my_access_token>"`
1. `tar -zxvf qrypt-security-0.10.2-ubuntu.tgz --strip-components=1 -C QryptSecurity`
1. `cmake . -B build`
1. `cmake --build build`
1. `./qrypt --help`

If googletest is installed on your system, you may add `-DBUILD_TESTS=ON` to your cmake command to enable an automated
validation suite which can be run with `./qrypt test`:
![test example](res/rest_run.png)
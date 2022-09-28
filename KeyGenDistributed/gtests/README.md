# Gtest Setup

## Test environment

The test commands shown in this tutorial should be run on an Ubuntu 20.04 system.

## Prerequisites (if you haven't setup Qrypt Security SDK on this host)
- A Qrypt Account. [Create an account for free](https://portal.qrypt.com/register)

## Setup (if you haven't setup Qrypt Security SDK on this host)
1. *Optional: If you have docker installed on the system (e.g. Mac OS), you could run gtest in an Ubuntu container instead of an Ubuntu desktop.*
    ```
    $ docker run --name qrypt_ubuntu -it --rm ubuntu:20.04 bash
    ```

1. Retrieve a token from the [Qrypt Portal](https://portal.qrypt.com/tokens).
    
    Create an environment variable **QRYPT_TOKEN** for the token. 
    ```
    $ export QRYPT_TOKEN="eyJhbGciOiJ........." >> ~/.bashrc
    ```
1. Install the development and network tools.
    ```
    $ apt-get update
    $ DEBIAN_FRONTEND="noninteractive" TZ="America/New_York" apt-get install -y cmake
    $ apt-get -y install git gcc g++ xxd libssl-dev libgtest-dev curl jq
    ```

1. Clone the [repo](https://github.com/QryptInc/qrypt-security-quickstarts-cpp) containing this quickstart to a local folder.
    ```
    $ git clone https://github.com/QryptInc/qrypt-security-quickstarts-cpp.git
    $ cd qrypt-security-quickstarts-cpp
    $ git checkout main
    ```
1. Download the Qrypt Security SDK from the [Qrypt Portal](https://portal.qrypt.com/downloads/sdk-downloads) for Ubuntu.
    ```
    curl -s $(curl -s https://quantumcryptogateway.blob.core.windows.net/sdk/sdk-languages.json | jq -r '.. |."downloadLink"? | select(. != null)') --output qrypt-security-ubuntu.tgz
    ```
1. Extract the Qrypt SDK into the /qrypt-security-quickstarts-cpp/KeyGenDistributed/lib/QryptSecurity folder
    ```
    $ tar -zxvf qrypt-security-ubuntu.tgz --strip-components=1 -C KeyGenDistributed/lib/QryptSecurity
    ```
    At this point you should be able to see the header files and libraries under KeyGenDistributed/lib/QryptSecurity.
    ```
    $ ls KeyGenDistributed/lib/QryptSecurity/
      include  lib  licenses
    ```

## To run the tests
Go to the gtests folder, build and run the tests.
```
$ cd KeyGenDistributed/gtests/
$ ./build.sh
$ build/KeyGenDistributedTests
```

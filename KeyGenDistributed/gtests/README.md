## Test environment

The test commands in this tutorial should be run on an Ubuntu system.

## Prerequisite
Go through [Quickstarts guide](https://docs.qrypt.com/sdk/quickstarts/cpp/keygendistributed/) and make sure that the qrypt token is ready.
```
$ echo $QRYPT_TOKEN
```

Install the gtest library.
```
$ apt-get update
$ apt-get -y install libgtest-dev
```

## To run the tests
Go to the gtests folder, build and run the tests.
```
$ cd KeyGenDistributed/gtests/
$ ./build.sh
$ build/KeyGenDistributedTests
```
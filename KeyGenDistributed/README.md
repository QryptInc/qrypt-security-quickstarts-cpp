# Quickstart: Distributed Key Generation

For full instructions on how to build this code sample from scratch, look at [TODO: link to portal, Quickstart: Distributed Key Generation]()

## Prerequisites
- A Qrypt Account. [Create an account for free](https://portal.qrypt.com/).
- Install [Visual Studio Code](https://code.visualstudio.com/).
- Install [CMake](https://cmake.org/).
- (Windows) Install git bash - comes with typical git install.

## Setup
1. Clone the repo containing this quickstart to a local folder on a Linux, Mac or Windows platform.
1. Retrieve a token from the [Qrypt Portal](https://portal.qrypt.com/) and create an environment variable **QRYPT_TOKEN** for it.
1. Download the Qrypt Security SDK for your platform.
1. Create a lib folder and extract the Qrypt Security SDK into it.

*Expected Folder structure*

    KeyGenDistributed
       /lib
           /QryptSecurity
               /bin (Windows)
               /include
               /lib
               /res

## Build
In a terminal, change to the KeyGenDistributed folder and enter the following to build the KeyGenDistibuted console application.

    ./build.sh --build_type=Debug

Upon a successful build, the KeyGenDistributed console application can be found in the following folder:

*Linux/Mac - KeyGenDistributed/build*

*Windows - KeyGenDistributed/build/Debug* 

Enter the following command for a complete set of build options:

    ./build.sh --help

## Run
### Run as Alice
This will display the shared key and write out the metadata file for Bob.

Change to the following folder:

*Linux/Mac - KeyGenDistributed/build*

*Windows - KeyGenDistributed/build/Debug* 

Enter the following command:

    ./KeyGenDistributed --user=alice --token=${QRYPT_TOKEN} --key-type=aes --metadata-filename=metadata.bin
 
### Run as Bob
This will consume the metadata file created by Alice above and display the shared key.

Make sure you are still in the folder as specified above and enter the following command:

    ./KeyGenDistributed --user=bob --token=${QRYPT_TOKEN} --metadata-filename=metadata.bin

## Debug
If you open the folder KeyGenDistributed In Visual Studio Code, you will find debug setups for running as Alice and Bob.



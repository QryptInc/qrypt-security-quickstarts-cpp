# Quickstart: Local Key Generation
For full instructions on how to build this code sample from scratch, look at [TODO: link to portal, Quickstart: Local Key Generation]()

## Prerequisites
- A Qrypt Account. [Create an account for free](https://portal.qrypt.com/).
- Install [Visual Studio Code](https://code.visualstudio.com/).
- Install [CMake](https://cmake.org/).
- (Windows) Install git bash - comes with typical git install.

# Setup
1. Clone the repo containing this quickstart to a local folder on a Linux, Mac or Windows platform.
1. Retrieve a token from the [Qrypt Portal](https://portal.qrypt.com/) and create an environment variable **QRYPT_TOKEN** for it.
1. Download the Qrypt Security SDK for your platform.
1. Create a lib folder and extract the Qrypt Security SDK into it.

*Expected Folder structure*

    KeyGenLocal
       /lib
           /QryptSecurity
               /bin (Windows)
               /include
               /lib
               /res

## Build
In a terminal, change to the KeyGenLocal folder and enter the following to build the KeyGenLocal console application.

    ./build.sh --build_type=Debug

Upon a successful build, the KeyGenLocal console application can be found in the following folder:

*Linux/Mac - KeyGenLocal/build*

*Windows - KeyGenLocal/build/Debug* 

Enter the following command for a complete set of build options:

    ./build.sh --help

## Run
This will display the locally generated AES key.

Change to the followng folder:

*Linux/Mac - KeyGenLocal/build*

*Windows - KeyGenLocal/build/Debug* 

Enter the following command:

    ./KeyGenLocal ${QRYPT_TOKEN}
 
## Debug
If you open the folder KeyGenLocal In Visual Studio Code, you will find a debug setup for KeyGenLocal.


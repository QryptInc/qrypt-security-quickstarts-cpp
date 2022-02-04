# Quickstart: Distributed Key Generation

For full instructions on how to build this code sample from scratch, look at [Quickstart: Distributed Key Generation](https://docs.qrypt.com/sdk/quickstarts/cpp/keygendistributed/).

## Prerequisites
- A Qrypt Account. [Create an account for free](https://portal.qrypt.com/).
- Install [CMake](https://cmake.org/).
- (Windows) Install git bash - comes with typical git install.
- (Optional) Install [Visual Studio Code](https://code.visualstudio.com/).

## Setup
1. Clone the repo containing this quickstart to a local folder on a Linux, Mac or Windows platform.
1. Retrieve a token from the [Qrypt Portal](https://portal.qrypt.com/) with the scope **KEYGEN**.
1. (Optional) Create an environment variable **QRYPT_TOKEN** for it. For simplicity, the commands below will be referencing a **QRYPT_TOKEN** environment variable but you can also just use the token direclty in the commands below.
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

To change to the KeyGenDistributed folder:
    
    vm@vm:~$ cd KeyGenDistributed

To make a (debug) build:
    
    vm@vm:~/KeyGenDistributed$ ./build.sh --build_type=Debug

To find the build folder (if it built successfully):

*For linux/mac*
        
    vm@vm:~/KeyGenDistributed$ ls -d build
    
*For windows*

    vm@vm:~/KeyGenDistributed$ ls -d build/Debug/

To see more build options:
    
    vm@vm:~/KeyGenDistributed$ ./build.sh --help
## Run
### Run as Alice
This will display the shared key and write out the metadata file for Bob.

To change to the KeyGenDistributed build folder:

*For linux/mac*
    
    vm@vm:~$ cd KeyGenDistributed/build

*for windows*

    vm@vm:~$ cd KeyGenDistributed/build/Debug

 
To run as Alice:

    ./KeyGenDistributed --user=alice --token=${QRYPT_TOKEN} --key-type=aes --metadata-filename=metadata.bin
 
### Run as Bob
This will consume the metadata file created by Alice above and display the shared key.

To change to the KeyGenDistributed build folder:

*For linux/mac*
    
    vm@vm:~$ cd KeyGenDistributed/build

*for windows*

    vm@vm:~$ cd KeyGenDistributed/build/Debug

 
To run as Alice:

    ./KeyGenDistributed --user=bob --token=${QRYPT_TOKEN} --metadata-filename=metadata.bin

## Debug
If you open the folder KeyGenDistributed In Visual Studio Code, you will find debug setups for running as Alice and Bob.



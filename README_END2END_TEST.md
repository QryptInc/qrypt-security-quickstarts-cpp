# End-to-End Test Setup
This tutorial demonstrates the steps to setup and run end-to-end tests manually. However, we note that a [docker version](demo/README.md) that automates these steps in a docker environment is also available.

In addition, this tutorial illustrates a two-device scenario (Alice and Bob). To test additional device(s), repeat Bob's setup and steps on another host. For example, to test a three-device scenario (e.g., Alice, Bob, Carol), repeat Bob's setup and steps on Carol's host.

## Test environment
The test commands shown in this tutorial should be run on an Ubuntu 20.04 system.


## Prerequisites
- A Qrypt Account. [Create an account for free](https://portal.qrypt.com/register)
- Open up 2 terminals for Alice and Bob respectively, on either the same or different hosts.

## Setup  (on both Alice's and Bob's hosts)
1. *Optional: If you have docker installed on the system (e.g. Mac OS), you could run Alice and Bob in Ubuntu containers instead of Ubuntu desktops.*
    ```
    docker run --name {alice/bob}_ubuntu -it --rm ubuntu:20.04 bash
    ```

1. Retrieve a token from the [Qrypt Portal](https://portal.qrypt.com/tokens).
    
    Create an environment variable **QRYPT_TOKEN** for the token.
    ```
    export QRYPT_TOKEN="eyJhbGciOiJ......"
    ```
    *Optional: to set QRYPT_TOKEN permanently for all future bash sessions, put it in ~/.bashrc*
    ```
    export QRYPT_TOKEN="eyJhbGciOiJ......" >> ~/.bashrc
    ```
1. Install the development and network tools.
    ```
    apt-get update
    DEBIAN_FRONTEND="noninteractive" TZ="America/New_York" apt-get install -y cmake git gcc g++ xxd libssl-dev libgtest-dev curl jq
    ```

1. Clone the [repo](https://github.com/QryptInc/qrypt-security-quickstarts-cpp) containing this quickstart to a local folder.
    ```
    git clone https://github.com/QryptInc/qrypt-security-quickstarts-cpp.git
    cd qrypt-security-quickstarts-cpp
    ```
1. Download the Qrypt Security SDK from the [Qrypt Portal](https://portal.qrypt.com/downloads/sdk-downloads) for Ubuntu.
    
1. Extract the Qrypt SDK into the /qrypt-security-quickstarts-cpp/KeyGenDistributed/lib/QryptSecurity folder
    ```
    tar -zxvf <sdk file> --strip-components=1 -C KeyGenDistributed/lib/QryptSecurity
    ```
    *Optional: At this point you should be able to see the header files and libraries under KeyGenDistributed/lib/QryptSecurity.*
    ```
    # Expected output:  include  lib  licenses
    ls KeyGenDistributed/lib/QryptSecurity/ 
    ```

1. Build the keygen and encryption command line tools.
    ```
    cd KeyGenDistributed
    ./build.sh --build_encrypt_tool
    ```
    
    *Optional: to make a debug build*
    ```
    ./build.sh --build_encrypt_tool --build_type=Debug
    ```

1. Setup SSH server and user - will be used for file transmission between Alice and Bob in the below tests.
    ```
    apt-get -y install openssh-server ufw sshpass net-tools
    ```
    ```
    service ssh start
    useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 ubuntu
    echo "ubuntu:ubuntu" | chpasswd
    ```

## Test commands (on Alice's host)
Alice generates AES/OTP keys and metadata files, encrypts the files, and sends the metadata and encrypted files to Bob.
```
# AES keygen and encryption
build/KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=aes --metadata-filename=aes_metadata.bin --key-filename=alice_aes.bin
build/EncryptTool --op=encrypt --key-type=aes --key-filename=alice_aes.bin --file-type=bitmap --input-filename=../files/tux.bmp --output-filename=aes_encrypted_tux.bmp
```

```
# OTP keygen and encryption
build/KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=otp --otp-len=$(stat -c%s ../files/sample.txt) --metadata-filename=otp_metadata.bin --key-filename=alice_otp.bin
build/EncryptTool --op=encrypt --key-type=otp --key-filename=alice_otp.bin --file-type=binary --input-filename=../files/sample.txt --output-filename=otp_encrypted_sample.bin
```

**Remarks:** To find Bob's IP, run `ifconfig eth0 | grep "inet " | awk '{print $2}'` on Bob's host.

```
# Send the metadata and encrypted files
sshpass -p "ubuntu" scp -o 'StrictHostKeyChecking no' aes_metadata.bin aes_encrypted_tux.bmp otp_metadata.bin otp_encrypted_sample.bin ubuntu@<Bob's IP>:/home/ubuntu
```

## Test commands (on Bob's host)
Bob recovers the keys using the metadata files, decrypts the files, and compares the decrypted files with the original ones.

*Optional: At this point Bob should have received the metadata and encrypted files from Alice.*
```
# Expected output:  aes_encrypted_tux.bmp  aes_metadata.bin  otp_encrypted_sample.bin  otp_metadata.bin
ls /home/ubuntu/
```

```
# AES keygen and decryption
build/KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=/home/ubuntu/aes_metadata.bin --key-filename=bob_aes.bin
build/EncryptTool --op=decrypt --key-type=aes --key-filename=bob_aes.bin --file-type=bitmap --input-filename=/home/ubuntu/aes_encrypted_tux.bmp --output-filename=aes_decrypted_tux.bmp
```

```
# OTP keygen and decryption
build/KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=/home/ubuntu/otp_metadata.bin --key-filename=bob_otp.bin
build/EncryptTool --op=decrypt --key-type=otp --key-filename=bob_otp.bin --file-type=binary --input-filename=/home/ubuntu/otp_encrypted_sample.bin --output-filename=otp_decrypted_sample.bin
```

```
# Verify the decrypted files
cmp ../files/tux.bmp aes_decrypted_tux.bmp
cmp ../files/sample.txt otp_decrypted_sample.bin
```
The decrypted files should be identical to the original ones.

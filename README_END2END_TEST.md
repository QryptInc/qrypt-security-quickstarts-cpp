## Test environment

The test commands shown in this tutorial should be run on an Ubuntu 20.04 system.

**Remarks:** This tutorial demonstrates the steps to setup and run end-to-end tests manually. However, we note that a [docker version](demo/README.md) that automates these steps in a docker environment is also available.

## Prerequisites
- A Qrypt Account. [Create an account for free](https://portal.qrypt.com/register)

## Setup  (on both Alice's and Bob's hosts)
1. Install Openssl and other development and network tools.
    ```
    $ apt-get update
    $ apt-get -y install libssl-dev
    $ apt-get -y install git cmake gcc g++ xxd libssl-dev libgtest-dev openssh-server ufw sshpass net-tools
    ```

1. *Optional: Setup SSH server and user - will be used for file transmission.*
    ```
    $ ufw allow ssh
    $ service ssh start
    $ useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 ubuntu
    $ echo "ubuntu:ubuntu" | chpasswd
    ```
1. Clone the [repo](https://github.com/QryptInc/qrypt-security-quickstarts-cpp) containing this quickstart to a local folder.
1. Retrieve a token from the [Qrypt Portal](https://portal.qrypt.com/tokens).
1. Create an environment variable **QRYPT_TOKEN** for the token. For simplicity, the commands below will be referencing a **QRYPT_TOKEN** environment variable but you can also just use the token directly in the commands below.
1. Download the Qrypt Security SDK from the [Qrypt Portal](https://portal.qrypt.com/downloads/sdk-downloads) for your platform.
1. Extract the Qrypt SDK.
    ```
    $ tar -zxvf qrypt-security-x.x.x-ubuntu.tgz
    ```
1. Copy the Qrypt SDK into the /qrypt-security-quickstarts-cpp/KeyGenDistributed/lib/QryptSecurity folder.
    ```
    $ cp -r qrypt-security-x.x.x-ubuntu/* /qrypt-security-quickstarts-cpp/KeyGenDistributed/lib/QryptSecurity

    Expected Folder structure:
        KeyGenDistributed
        /lib
            /QryptSecurity
                /include
                /lib
    ```

## Build
Build the keygen and encryption command line tools.
```
$ cd KeyGenDistributed
$ ./build.sh --build_encrypt_tool
```

## Test commands (on Alice's host)
Alice generates the AES key and metadata.
```
$ build/KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=aes --metadata-filename=metadata.bin --key-filename=alice_aes.bin
```

Alice encrypts the bmp image file using the AES key.
```
$ build/EncryptTool --op=encrypt --key-type=aes --key-filename=alice_aes.bin --file-type=bitmap --input-filename=../files/tux.bmp --output-filename=aes_encrypted_tux.bmp
```

Alice sends the metadata and the encrypted image file to Bob. 

**Remarks:** To find Bob's IP, run `ifconfig eth0 | grep "inet " | awk '{print $2}'` on Bob's host.

Below is a sample command that sends the files to Bob's host using scp.
```
$ sshpass -p "ubuntu" scp -o 'StrictHostKeyChecking no' metadata.bin aes_encrypted_tux.bmp ubuntu@<Bob's IP>:/home/ubuntu
```

## Test commands (on Bob's host)
Bob recovers the AES key using the metadata file.
```
$ build/KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=/home/ubuntu/metadata.bin --key-filename=bob_aes.bin
```

Bob decrypts the bmp image file using the AES key.
```
$ build/EncryptTool --op=decrypt --key-type=aes --key-filename=bob_aes.bin --file-type=bitmap --input-filename=/home/ubuntu/aes_encrypted_tux.bmp --output-filename=aes_decrypted_tux.bmp
```

Bob verifies that the decrypted image file matches the original file.
```
$ cmp ../files/tux.bmp aes_decrypted_tux.bmp
```

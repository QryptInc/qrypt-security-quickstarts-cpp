## Test environment

The test commands shown in this tutorial should be run on an Ubuntu 20.04 system.

**Remarks:** This tutorial demonstrates the steps to setup and run end-to-end tests manually. However, we note that a [docker version](demo/README.md) that automates these steps in a docker environment is also available.

## Prerequisites
- A Qrypt Account. [Create an account for free](https://portal.qrypt.com/register)

## Setup  (on both Alice's and Bob's hosts)
1. *Optional: If you have docker installed in the system (e.g. MacOS), you could run Alice and Bob in Ubuntu containers instead of Ubuntu desktops.*
    ```
    $ docker run --name {alice/bob}_ubuntu -it --rm ubuntu:20.04 bash
    ```

1. Retrieve a token from the [Qrypt Portal](https://portal.qrypt.com/tokens).
    
    Create an environment variable **QRYPT_TOKEN** for the token. 
    ```
    $ export QRYPT_TOKEN="eyJhbGciOiJ........." >> ~/.bashrc
    ```
1. Install Openssl and other development and network tools.
    ```
    $ apt-get update
    $ apt-get -y install git cmake gcc g++ xxd libssl-dev libgtest-dev curl jq
    ```

1. *Optional: Setup SSH server and user - will be used for file transmission.*
    ```
    $ apt-get -y install openssh-server ufw sshpass net-tools
    $ service ssh start
    $ useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 ubuntu
    $ echo "ubuntu:ubuntu" | chpasswd
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
1. Build the keygen and encryption command line tools.
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
At this point Bob should have received the metadata and the encrypted image file from Alice.
```
$ ls /home/ubuntu/
aes_encrypted_tux.bmp  metadata.bin
```

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

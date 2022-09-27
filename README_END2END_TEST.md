## Test environment

The test commands shown in this tutorial should be run on an Ubuntu system.

**Remarks:** This tutorial demonstrates the steps to setup and run end-to-end tests manually. However, we note that a [docker version](demo/README.md) that automates these steps in a docker environment is also available.

## Prerequisite (on both Alice's and Bob's hosts)
Go through [Quickstarts guide](https://docs.qrypt.com/sdk/quickstarts/cpp/keygendistributed/) and make sure that the qrypt token is ready.
```
$ echo $QRYPT_TOKEN
```

Install Openssl and other development and network tools.
```
$ apt-get update
$ apt-get -y install libssl-dev
$ apt-get -y install git cmake gcc g++ xxd libssl-dev libgtest-dev openssh-server ufw sshpass net-tools
```

Setup SSH server and user - will be used for file transmission.
```
ufw allow ssh
service ssh start
useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 ubuntu
echo "ubuntu:ubuntu" | chpasswd
```

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
$ diff ../files/tux.bmp aes_decrypted_tux.bmp
```

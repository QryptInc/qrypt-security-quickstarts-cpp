# Container Demo
This demo demonstrates the Quickstarts and end-to-end tests setup in a docker ennvironment. It clones the quickstarts repo from the github, sets up the folder structure, downloads the SDK zipped file, builds the test codes, and creates Alice's and Bob's containers for end-to-end tests.

## Test environment
The testbed in this demo will be established in Ubuntu containers, so the below commands can be run from any OS with docker installed. They have been tested on MacOS.

## Prerequisites
1. Have git and docker installed.
1. Retrieve a token from the [Qrypt Portal](https://portal.qrypt.com/tokens).
1. Create an environment variable **QRYPT_TOKEN** for the token.
    ```
    export QRYPT_TOKEN="eyJhbGciOiJ......"
    ```
    *Optional: to set QRYPT_TOKEN permanently for all future bash sessions, put it in ~/.bashrc*
    ```
    export QRYPT_TOKEN="eyJhbGciOiJ......" >> ~/.bashrc
    ```

## Bring up the testbed
```
git clone https://github.com/QryptInc/qrypt-security-quickstarts-cpp.git
cd qrypt-security-quickstarts-cpp
git checkout main
cd demo
docker-compose down -v --rmi all --remove-orphans
QRYPT_TOKEN=$QRYPT_TOKEN docker-compose up -d
```

## Run Alice and Bob tests (2 options: automatically and manually)

### [Option 1 - Automatically] run Alice and Bob tests from the host
```
./run_alice_bob.sh
```

### [Option 2 - Manually] run Alice and Bob tests from their containers

- Open up another 2 terminals and follow the below instructions in each terminal.

#### Terminal - Alice

Alice generates AES/OTP keys and metadata files, encrypts the files, and sends the metadata and encrypted files to Bob.

```
# Enter Alice's container
docker exec -it alice_container bash
```

```
# AES keygen and encryption
KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=aes --metadata-filename=aes_metadata.bin --key-filename=alice_aes.bin
EncryptTool --op=encrypt --key-type=aes --key-filename=alice_aes.bin --file-type=bitmap --input-filename=/workspace/files/tux.bmp --output-filename=aes_encrypted_tux.bmp
```

```
# OTP keygen and encryption
KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=otp --otp-len=$(stat -c%s /workspace/files/sample.txt) --metadata-filename=otp_metadata.bin --key-filename=alice_otp.bin
EncryptTool --op=encrypt --key-type=otp --key-filename=alice_otp.bin --file-type=binary --input-filename=/workspace/files/sample.txt --output-filename=otp_encrypted_sample.bin
```

```
# Send the metadata and encrypted files
sshpass -p "ubuntu" scp -o 'StrictHostKeyChecking no' aes_metadata.bin aes_encrypted_tux.bmp otp_metadata.bin otp_encrypted_sample.bin ubuntu@bob:/home/ubuntu
```

#### Terminal - Bob
Bob recovers the keys using the metadata files, decrypts the files, and compares the decrypted files with the original ones.
```
# Enter Bob's container
docker exec -it bob_container bash
```
```
# AES keygen and decryption
KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=aes_metadata.bin --key-filename=bob_aes.bin
EncryptTool --op=decrypt --key-type=aes --key-filename=bob_aes.bin --file-type=bitmap --input-filename=aes_encrypted_tux.bmp --output-filename=aes_decrypted_tux.bmp
```
```
# OTP keygen and decryption
KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=otp_metadata.bin --key-filename=bob_otp.bin
EncryptTool --op=decrypt --key-type=otp --key-filename=bob_otp.bin --file-type=binary --input-filename=otp_encrypted_sample.bin --output-filename=otp_decrypted_sample.bin
```
```
# Verify the decrypted files
cmp /workspace/files/tux.bmp aes_decrypted_tux.bmp
cmp /workspace/files/sample.txt otp_decrypted_sample.bin
```
The decrypted files should be identical to the original ones.

## Bonus - gtest
This is not related to this demo, but it's worth mentioning that you could run gtest in either Alice's or Bob's container easily.
```
# Enter Alice's container - if you haven't done so
docker exec -it alice_container bash
```

```
# Run the gtests
cd /workspace/KeyGenDistributed/gtests/
./build.sh
build/KeyGenDistributedTests
```

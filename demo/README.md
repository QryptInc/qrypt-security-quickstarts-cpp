This demo demonstrates the Quickstarts and end to end tests setup in a docker ennvironment. It clones the quickstarts repo from the github, sets up the folder structure, downloads the SDK zipped file, builds the test codes, and creates Alice's and Bob's containers for end to end tests.

## Test environment
The testbed in this demo will be established in Ubuntu containers, so the below commands can be run from any OS with docker installed. They have been tested on MacOS.

## Preperation
- Have git and docker installed.

## Bring up the testbed
Make sure that the qrypt token is ready. 
```
$ echo $MY_QRYPT_TOKEN
```

Bring up the testbed.
```
$ git clone https://hard-carbon.visualstudio.com/Qrypt/_git/qrypt-security-demo-cpp
$ cd qrypt-security-demo-cpp/demo
$ docker-compose down -v --rmi all --remove-orphans
$ QRYPT_TOKEN=$MY_QRYPT_TOKEN docker-compose up -d
```

## Run Alice and Bob tests (2 options: automatically and manually)

### [Automatically] run Alice and Bob tests from the host
```
$ ./run_alice_bob.sh
```

### [Manually] run Alice and Bob tests from their containers

- Open up another 2 terminals and follow the below instructions in each terminal.

##### Terminal - Alice
Alice generates AES key and the metadata, encrypts the image, and then sends the metadata and the encrypted image to Bob.
```
$ docker exec -it alice_container bash
$ KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=aes --metadata-filename=metadata.bin --key-filename=alice_aes.bin
$ EncryptTool --op=encrypt --key-type=aes --key-filename=alice_aes.bin --file-type=bitmap --input-filename=/workspace/files/tux.bmp --output-filename=aes_encrypted_tux.bmp
$ sshpass -p "ubuntu" scp -o 'StrictHostKeyChecking no' metadata.bin aes_encrypted_tux.bmp ubuntu@bob:/home/ubuntu
```

##### Terminal - Bob
Bob recovers the AES key using the metadata, decrypts the image, and compare the decrypted image with the original one.
```
$ docker exec -it bob_container bash
$ KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=metadata.bin --key-filename=bob_aes.bin
$ EncryptTool --op=decrypt --key-type=aes --key-filename=bob_aes.bin --file-type=bitmap --input-filename=aes_encrypted_tux.bmp --output-filename=aes_decrypted_tux.bmp
$ cmp /workspace/files/tux.bmp aes_decrypted_tux.bmp
```


## Bonus - gtest
This is not related to this demo, but it's worth mentiong that you could run gtest in either Alice's or Bob's container easily.
```
$ docker exec -it alice_container bash
root@alice:/home/ubuntu$ cd /workspace/KeyGenDistributed/gtests/
root@alice:/workspace/KeyGenDistributed/gtests$ ./build.sh
root@alice:/workspace/KeyGenDistributed/gtests$ build/KeyGenDistributedTests
```

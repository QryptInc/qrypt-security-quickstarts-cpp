## Multi-device demonstration using Docker-Compose
The `compose` subdirectory contains a docker-compose.yml and sample scripts for simulating true distributed keygen on two separate systems.

1. On the [Qrypt portal](https://portal.qrypt.com/register), register a free account and create a personal access token for keygen.
1. Export the token to your environment: `export QRYPT_TOKEN="eyJhbGciOiJ......"`
1. Bring up the "Alice" and "Bob" demonstration containers:
    ```
    git clone https://github.com/QryptInc/qrypt-security-quickstarts-cpp.git
    cd qrypt-security-quickstarts-cpp/compose
    QRYPT_TOKEN=$QRYPT_TOKEN docker-compose up -d
    ```
1. Enter the "Alice" container, generate a key, encrypt the sample text, and transfer the ciphertext + metadata to "Bob":
    ```
    # Enter Alice's container
    docker exec -it alice_container bash
    ```
    ```
    # OTP generation and encryption
    qrypt generate --token=$QRYPT_TOKEN --key-len=$(stat -c%s /workspace/files/sample.txt) --key-filename=key.dat
    qrypt encrypt --input-filename=/workspace/files/sample.txt --key-filename=key.dat --output-filename=ciphertext.dat
    ```
    ```
    # Send the OTP metadata and encrypted files to Bob
    sshpass -p "ubuntu" scp -o 'StrictHostKeyChecking no' meta.dat ciphertext.dat ubuntu@bob:/home/ubuntu
    ```
1. Enter the "Bob" container, replicate the key, decrypt the ciphertext, and compare the result with the original sample text:
    ```
    # Enter Bob's container
    docker exec -it bob_container bash
    ```
    ```
    # OTP replication and decryption
    qrypt replicate --token=$QRYPT_TOKEN --key-len=$(stat -c%s ciphertext.dat) --key-filename=key.dat
    qrypt decrypt --input-filename=ciphertext.dat --key-filename=key.dat --output-filename=decrypted.txt
    ```
    ```
    # Verify the OTP decrypted files
    cmp /workspace/files/sample.txt decrypted.txt
    ```
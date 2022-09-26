# Clean up the test files
docker exec alice_container sh -c 'rm -rf /home/ubuntu/*'
docker exec bob_container sh -c 'rm -rf /home/ubuntu/*'

printf "\nAlice generates AES key and metadata."
docker exec alice_container sh -c 'KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=aes --metadata-filename=metadata.bin --key-filename=alice_aes.bin'

printf "\nAlice encrypts the image file using the AES key."
docker exec alice_container sh -c 'EncryptTool --op=encrypt --key-type=aes --key-filename=alice_aes.bin --file-type=bitmap --input-filename=/workspace/files/tux.bmp --output-filename=aes_encrypted_tux.bmp'

printf "\nAlice sends the metadata and encrypted image to Bob.\n"
docker exec alice_container sh -c 'sshpass -p "ubuntu" scp -o "StrictHostKeyChecking no" metadata.bin aes_encrypted_tux.bmp ubuntu@bob:/home/ubuntu'

printf "\nBob recovers the AES key using the metadata."
docker exec bob_container sh -c 'KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=metadata.bin --key-filename=bob_aes.bin'

printf "\nBob decrypts the encrypted image file using the AES key.\n"
docker exec bob_container sh -c 'EncryptTool --op=decrypt --key-type=aes --key-filename=bob_aes.bin --file-type=bitmap --input-filename=aes_encrypted_tux.bmp --output-filename=aes_decrypted_tux.bmp'

# Compare the decrypted file with the original file
if docker exec bob_container sh -c 'cmp -s /workspace/files/tux.bmp aes_decrypted_tux.bmp' ; then
   printf "\nVerified: the decrypted file matches the original one.\n"
else
   printf "\nError: the decrypted file is different from the original one.\n"
fi

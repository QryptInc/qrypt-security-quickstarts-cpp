#!/usr/bin/bash

# exit when any command fails
set -e

BOLD=$(tput bold) # bold 
NB=$(tput sgr0)   # not bold
prefix="${BOLD}*****${NB}"

ORIGINAL_TEXT_FILE="/workspace/files/sample.txt"
ORIGINAL_IMAGE_FILE="/workspace/files/tux.bmp"
OTP_KEY="alice_otp.bin"
OTP_METADATA="otp_metadata.bin"
OTP_ENCRYPTED_TEXT_FILE="otp_encrypted_sample.bin"
AES_KEY="alice_aes.bin"
AES_METADATA="aes_metadata.bin"
AES_BIN_ENCRYPTED_IMAGE_FILE="aes_encrypted_tux.bin"
AES_BMP_ENCRYPTED_IMAGE_FILE="aes_encrypted_tux.bmp"

printf "\n$prefix Alice generates an OTP key and metadata.\n"
eval 'KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=otp --otp-len=$(stat -c%s $ORIGINAL_TEXT_FILE) --metadata-filename=$OTP_METADATA --key-filename=alice_otp.bin'

printf "\n$prefix Alice encrypts $ORIGINAL_TEXT_FILE in binary format using the OTP key ($OTP_KEY).\n"
eval 'EncryptTool --op=encrypt --key-type=otp --key-filename=$OTP_KEY --file-type=binary --input-filename=$ORIGINAL_TEXT_FILE --output-filename=$OTP_ENCRYPTED_TEXT_FILE'

printf "\n$prefix Alice generates an AES key ($AES_KEY) and metadata ($AES_METADATA).\n"
eval 'KeyGenDistributed --user=alice --token=$QRYPT_TOKEN --key-type=aes --metadata-filename=$AES_METADATA --key-filename=$AES_KEY'

printf "\n$prefix Alice encrypts $ORIGINAL_IMAGE_FILE in binary format using the AES key ($AES_KEY).\n"
eval 'EncryptTool --op=encrypt --key-type=aes --key-filename=$AES_KEY --file-type=binary --input-filename=$ORIGINAL_IMAGE_FILE --output-filename=$AES_BIN_ENCRYPTED_IMAGE_FILE'

printf "\n$prefix Alice encrypts $ORIGINAL_IMAGE_FILE in bmp image format using the AES key ($AES_KEY).\n"
eval 'EncryptTool --op=encrypt --key-type=aes --key-filename=$AES_KEY --file-type=bitmap --input-filename=$ORIGINAL_IMAGE_FILE --output-filename=$AES_BMP_ENCRYPTED_IMAGE_FILE'

printf "\n$prefix Alice sends the metadata and encrypted files to Bob.\n"
eval 'sshpass -p "ubuntu" scp -o "StrictHostKeyChecking no" $OTP_METADATA $AES_METADATA $OTP_ENCRYPTED_TEXT_FILE $AES_BIN_ENCRYPTED_IMAGE_FILE $AES_BMP_ENCRYPTED_IMAGE_FILE ubuntu@bob:/home/ubuntu'

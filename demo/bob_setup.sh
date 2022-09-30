#!/usr/bin/bash

# exit when any command fails
set -e

BOLD=$(tput bold) # bold 
NB=$(tput sgr0)   # not bold
prefix="${BOLD}*****${NB}"

ORIGINAL_TEXT_FILE="/workspace/files/sample.txt"
ORIGINAL_IMAGE_FILE="/workspace/files/tux.bmp"

AES_KEY="bob_aes.bin"
AES_METADATA="aes_metadata.bin"
AES_BIN_ENCRYPTED_IMAGE_FILE="aes_encrypted_tux.bin"
AES_BIN_DECRYPTED_IMAGE_FILE="aes_decrypted_tux.bin"
AES_BMP_ENCRYPTED_IMAGE_FILE="aes_encrypted_tux.bmp"
AES_BMP_DECRYPTED_IMAGE_FILE="aes_decrypted_tux.bmp"

OTP_KEY="bob_otp.bin"
OTP_METADATA="otp_metadata.bin"
OTP_ENCRYPTED_TEXT_FILE="otp_encrypted_sample.bin"
OTP_DECRYPTED_TEXT_FILE="otp_decrypted_sample.bin"

# Compare the decrypted files with the original file
function compare {
   ORIGINAL=$1
   DECRYPTED=$2

   printf "\n$prefix Verifying the decrypted file ($DECRYPTED).\n"

   if eval 'cmp -s $ORIGINAL $DECRYPTED' ; then
      printf "[Verified] the decrypted file ($DECRYPTED) matches the original one ($ORIGINAL).\n"
   else
      printf "[Error] the decrypted file ($DECRYPTED) is different from the original one ($ORIGINAL).\n"
      exit 1
   fi
}

printf "\n$prefix Bob recovers the AES key using the metadata ($AES_METADATA).\n"
eval 'KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=$AES_METADATA --key-filename=$AES_KEY'

printf "\n$prefix Bob decrypts $AES_BIN_ENCRYPTED_IMAGE_FILE using the AES key ($AES_KEY).\n"
eval 'EncryptTool --op=decrypt --key-type=aes --key-filename=$AES_KEY --file-type=binary --input-filename=$AES_BIN_ENCRYPTED_IMAGE_FILE --output-filename=$AES_BIN_DECRYPTED_IMAGE_FILE'

printf "\n$prefix Bob decrypts $AES_BMP_ENCRYPTED_IMAGE_FILE using the AES key ($AES_KEY).\n"
eval 'EncryptTool --op=decrypt --key-type=aes --key-filename=$AES_KEY --file-type=bitmap --input-filename=$AES_BMP_ENCRYPTED_IMAGE_FILE --output-filename=$AES_BMP_DECRYPTED_IMAGE_FILE'

printf "\n$prefix Bob recovers the OTP key using the metadata ($OTP_METADATA).\n"
eval 'KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=$OTP_METADATA --key-filename=$OTP_KEY'

printf "\n$prefix Bob decrypts $OTP_ENCRYPTED_TEXT_FILE using the OTP key ($OTP_KEY).\n"
eval 'EncryptTool --op=decrypt --key-type=otp --key-filename=$OTP_KEY --file-type=binary --input-filename=$OTP_ENCRYPTED_TEXT_FILE --output-filename=$OTP_DECRYPTED_TEXT_FILE'

compare $ORIGINAL_IMAGE_FILE $AES_BMP_DECRYPTED_IMAGE_FILE
compare $ORIGINAL_IMAGE_FILE $AES_BIN_DECRYPTED_IMAGE_FILE
compare $ORIGINAL_TEXT_FILE $OTP_DECRYPTED_TEXT_FILE

printf "\n"

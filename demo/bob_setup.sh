#!/usr/bin/bash

# exit when any command fails
set -e

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

function print_header {
    TEXT=$1
    prefix="\n##################################################\n#"
    suffix="\n##################################################\n\n"

    printf "$prefix $TEXT $suffix"
}

# Compare the decrypted files with the original file
function compare {
   ORIGINAL=$1
   DECRYPTED=$2

   print_header "Verifying the decrypted file ($DECRYPTED)."

   if eval 'cmp -s $ORIGINAL $DECRYPTED' ; then
      printf "[Verified] the decrypted file ($DECRYPTED) matches the original one ($ORIGINAL).\n"
   else
      printf "[Error] the decrypted file ($DECRYPTED) is different from the original one ($ORIGINAL).\n"
      exit 1
   fi
}

printf "\n=================================================="
printf "\n=========== Bob's Keygen and Decryption =========="
printf "\n==================================================\n"

print_header "Bob recovers the AES key using the metadata ($AES_METADATA)."
eval 'KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=$AES_METADATA --key-filename=$AES_KEY'

print_header "Bob decrypts $AES_BIN_ENCRYPTED_IMAGE_FILE using the AES key ($AES_KEY)."
eval 'EncryptTool --op=decrypt --key-type=aes --key-filename=$AES_KEY --file-type=binary --input-filename=$AES_BIN_ENCRYPTED_IMAGE_FILE --output-filename=$AES_BIN_DECRYPTED_IMAGE_FILE'

print_header "Bob decrypts $AES_BMP_ENCRYPTED_IMAGE_FILE using the AES key ($AES_KEY)."
eval 'EncryptTool --op=decrypt --key-type=aes --key-filename=$AES_KEY --file-type=bitmap --input-filename=$AES_BMP_ENCRYPTED_IMAGE_FILE --output-filename=$AES_BMP_DECRYPTED_IMAGE_FILE'

print_header "Bob recovers the OTP key using the metadata ($OTP_METADATA)."
eval 'KeyGenDistributed --user=bob --token=$QRYPT_TOKEN --metadata-filename=$OTP_METADATA --key-filename=$OTP_KEY'

print_header "Bob decrypts $OTP_ENCRYPTED_TEXT_FILE using the OTP key ($OTP_KEY)."
eval 'EncryptTool --op=decrypt --key-type=otp --key-filename=$OTP_KEY --file-type=binary --input-filename=$OTP_ENCRYPTED_TEXT_FILE --output-filename=$OTP_DECRYPTED_TEXT_FILE'


printf "\n=================================================="
printf "\n======= Verify Bob's Keygen and Decryption ======="
printf "\n==================================================\n"

compare $ORIGINAL_IMAGE_FILE $AES_BMP_DECRYPTED_IMAGE_FILE
compare $ORIGINAL_IMAGE_FILE $AES_BIN_DECRYPTED_IMAGE_FILE
compare $ORIGINAL_TEXT_FILE $OTP_DECRYPTED_TEXT_FILE

printf "\n"

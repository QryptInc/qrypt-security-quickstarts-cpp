# exit when any command fails
set -e

# Clean up the test files
docker exec -t alice_container sh -c 'rm -rf /home/ubuntu/*'
docker exec -t bob_container sh -c 'rm -rf /home/ubuntu/*'

# Run Alice and Bob tests
docker exec -t alice_container sh -c '/workspace/files/alice_setup.sh'
docker exec -t bob_container sh -c '/workspace/files/bob_setup.sh'

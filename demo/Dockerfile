FROM ubuntu:20.04

ARG persona=docker
ENV PERSONA=$persona

ARG qrypt_token
ENV QRYPT_TOKEN=$qrypt_token

# Beautify the command prompt
RUN echo 'export PS1="\[\033[36m\]\u\[\033[m\]@\[\033[32m\]${PERSONA}:\[\033[33;1m\]\w\[\033[m\]\$ "' >> /root/.bashrc
RUN echo 'export CLICOLOR=1' >> /root/.bashrc
RUN echo 'export LSCOLORS=ExFxBxDxCxegedabagacad' >> /root/.bashrc

# Set timezone to avoid user interaction when installing cmake
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime
RUN echo $TZ > /etc/timezone

RUN apt-get update
RUN apt-get -y install git cmake gcc g++ xxd libssl-dev libgtest-dev openssh-server ufw sshpass curl jq

# Setup SSH
EXPOSE 22
RUN service ssh start
RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 ubuntu
RUN echo "ubuntu:ubuntu" | chpasswd

# Copy the sample files and quickstarts codes for testing
COPY files/ /workspace/files/
COPY demo/${PERSONA}_setup.sh /workspace/files/
COPY SDK_sha256sum /workspace/files/
COPY KeyGenDistributed/ /workspace/KeyGenDistributed/
WORKDIR /workspace/KeyGenDistributed

# Download and build SDK
RUN sdk_url=$(curl -s https://quantumcryptogateway.blob.core.windows.net/sdk/sdk-languages.json | jq -r '.. |."downloadLink"? | select(. != null)') && \
    sdk_file='qrypt-security-ubuntu.tgz' && \
    curl -s $sdk_url --output $sdk_file && \
    echo "$(cat /workspace/files/SDK_sha256sum) $sdk_file" > $sdk_file.sha256sum && \
    if ! (sha256sum --check $sdk_file.sha256sum --status); then echo "Error: unexpected checksum"; exit 1; fi && \
    tar -zxvf $sdk_file --strip-components=1 -C lib/QryptSecurity && \
    rm -rf $sdk_file
RUN ./build.sh --build_encrypt_tool
ENV PATH="${PATH}:/workspace/KeyGenDistributed/build"

WORKDIR /home/ubuntu

CMD ["/usr/sbin/sshd","-D"]
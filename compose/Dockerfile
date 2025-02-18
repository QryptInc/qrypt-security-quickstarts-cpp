FROM ubuntu:22.04

ARG persona=docker
ENV PERSONA=$persona

ARG qrypt_token
ENV QRYPT_TOKEN=$qrypt_token

# Beautify the command prompt
RUN echo 'export PS1="\[\033[36m\]\u\[\033[m\]@\[\033[32m\]${PERSONA}:\[\033[33;1m\]\w\[\033[m\]\$ "' >> /root/.bashrc
RUN echo 'export CLICOLOR=1' >> /root/.bashrc
RUN echo 'export LSCOLORS=ExFxBxDxCxegedabagacad' >> /root/.bashrc

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install git cmake gcc g++ xxd libssl-dev libgtest-dev libcurl4-openssl-dev openssh-server ufw sshpass curl jq

# Setup SSH
EXPOSE 22
RUN service ssh start
RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 ubuntu
RUN echo "ubuntu:ubuntu" | chpasswd

# Copy the sample files and quickstarts codes for testing
COPY . /workspace/
WORKDIR /workspace/

# Download SDK
RUN sdk_file='qrypt-security-ubuntu.tgz' && \
    curl -s https://qryptogateway.qrypt.com/sdk/cpp/v0.11.23/qrypt-security-0.11.23-ubuntu.tgz --output $sdk_file && \
    tar -zxvf $sdk_file --strip-components=1 -C QryptSecurity && \
    rm -rf $sdk_file

# Build SDK
RUN rm -rf build
RUN cmake . -B build
RUN cmake --build build
ENV PATH="${PATH}:/workspace/build"

WORKDIR /home/ubuntu

CMD ["/usr/sbin/sshd","-D"]
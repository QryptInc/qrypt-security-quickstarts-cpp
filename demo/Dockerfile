FROM ubuntu:20.04

ARG persona=docker
ENV PERSONA=$persona

ARG qrypt_token
ENV QRYPT_TOKEN=$qrypt_token

# Beautify the command prompt
RUN echo 'export PS1="\[\033[36m\]\u\[\033[m\]@\[\033[32m\]$PERSONA:\[\033[33;1m\]\w\[\033[m\]\$ "' >> /root/.bashrc
RUN echo 'export CLICOLOR=1' >> /root/.bashrc
RUN echo 'export LSCOLORS=ExFxBxDxCxegedabagacad' >> /root/.bashrc

ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime
RUN echo $TZ > /etc/timezone

RUN apt-get update
RUN apt-get -y install git cmake gcc g++ xxd libssl-dev libgtest-dev openssh-server ufw sshpass

# Setup SSH
RUN ufw allow ssh
EXPOSE 22
RUN service ssh start
RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 ubuntu
RUN echo "ubuntu:ubuntu" | chpasswd

# Copy the bmp image file and quickstarts codes for testing
COPY files/ /workspace/files/
COPY KeyGenDistributed/ /workspace/KeyGenDistributed/
WORKDIR /workspace/KeyGenDistributed

# Download and build SDK
RUN wget https://qrypt.azureedge.net/sdk/cpp/v0.6.4/qrypt-security-0.6.4-ubuntu.tgz -O /home/ubuntu/qrypt-security-0.6.4-ubuntu.tgz
RUN tar -xvf /home/ubuntu/qrypt-security-0.6.4-ubuntu.tgz --strip-components=1 -C lib/QryptSecurity
RUN rm -rf /home/ubuntu/qrypt-security-0.6.4-ubuntu.tgz
RUN ./build.sh --build_encrypt_tool
ENV PATH="${PATH}:/workspace/KeyGenDistributed/build"

WORKDIR /home/ubuntu

CMD ["/usr/sbin/sshd","-D"]
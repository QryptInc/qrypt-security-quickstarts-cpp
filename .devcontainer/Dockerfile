FROM --platform=linux/amd64 ubuntu:22.04

# Beautify the command prompt
RUN echo 'export PS1="\[\033[36m\]\u\[\033[m\]@\[\033[32m\]QRYPT:\[\033[33;1m\]\w\[\033[m\]\$ "' >> /root/.bashrc
RUN echo 'export CLICOLOR=1' >> /root/.bashrc
RUN echo 'export LSCOLORS=ExFxBxDxCxegedabagacad' >> /root/.bashrc

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install git cmake gcc g++ xxd libssl-dev libgtest-dev libcurl4-openssl-dev openssh-server ufw sshpass curl jq

# Install flask
RUN apt -y install python3-pip
RUN python3 -m pip install flask

# Install github CLI
RUN curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg && \
    chmod go+r /usr/share/keyrings/githubcli-archive-keyring.gpg && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | tee /etc/apt/sources.list.d/github-cli.list > /dev/null && \
    apt update && \
    apt install -y gh

# Install openssl 3.1+
RUN git clone git://git.openssl.org/openssl.git
RUN cd openssl && \
    git checkout openssl-3.1.4 && \
    ./config && \
    make && \
    make install && \
    echo "/usr/local/lib64" > /etc/ld.so.conf.d/openssl.conf && \
    ldconfig && \
    cd .. && \
    rm -rf openssl
RUN openssl version

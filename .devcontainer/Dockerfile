FROM ubuntu:20.04

# Beautify the command prompt
RUN echo 'export PS1="\[\033[36m\]\u\[\033[m\]@\[\033[32m\]QRYPT:\[\033[33;1m\]\w\[\033[m\]\$ "' >> /root/.bashrc
RUN echo 'export CLICOLOR=1' >> /root/.bashrc
RUN echo 'export LSCOLORS=ExFxBxDxCxegedabagacad' >> /root/.bashrc

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install git cmake gcc g++ xxd libssl-dev libgtest-dev openssh-server ufw sshpass curl jq

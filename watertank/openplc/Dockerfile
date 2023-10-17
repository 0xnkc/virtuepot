FROM ubuntu:xenial

USER root
WORKDIR /root/

RUN apt-get update
RUN apt-get install -y build-essential \
    libpcap-dev \
    libdnet-dev \
    libevent-dev \
    libpcre3-dev \
    make \
    bzip2 \
    nmap \
    psmisc \
    libtool \
    libdumbnet-dev \
    zlib1g-dev \
    rrdtool \
    net-tools \
    git-core \
    libreadline-dev \
    libedit-dev \
    bison \
    flex \
    farpd \
    lftp \
    iputils-ping \
    sudo \
    automake \
    sqlite3


WORKDIR /home/
RUN git clone https://github.com/DataSoft/Honeyd.git
RUN cd Honeyd \
    && ./autogen.sh \
    && ./configure \
    && make \
    && make install





WORKDIR /home/
ADD honeyd.conf .
# RUN git clone https://github.com/thiagoralves/OpenPLC_v3.git
# WORKDIR /home/OpenPLC_v3/
# RUN ls -al install.sh
# RUN sudo chmod +x install.sh
# RUN sudo ./install.sh linux

RUN sudo apt-get install -y build-essential pkg-config bison flex autoconf automake libtool make nodejs git
RUN git clone https://github.com/thiagoralves/OpenPLC_v2
WORKDIR /home/OpenPLC_v2/
RUN sudo chmod +x ./build.sh
RUN printf "n\n1\n" | ./build.sh


EXPOSE 502
EXPOSE 8080


# CMD  nohup honeyd -d -f /home/honeyd.conf &> honeyd.log; sudo nodejs /OpenPLC_v2/server.js
CMD sudo nodejs server.js ;nohup honeyd -d -f /app/honeyd.conf &> honeyd.log
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


COPY signatures.sh /app/signatures.sh
WORKDIR /app
ADD honeyd.conf /app/honeyd.conf
RUN chmod +x signatures.sh && ./signatures.sh

WORKDIR /home/

# RUN git clone https://github.com/thiagoralves/OpenPLC_v3.git
# WORKDIR /home/OpenPLC_v3/
# RUN ls -al install.sh
# RUN sudo chmod +x install.sh
# RUN sudo ./install.sh linux

RUN sudo apt-get install -y build-essential pkg-config bison flex autoconf automake libtool make nodejs git
RUN git clone https://github.com/thiagoralves/OpenPLC_v2
WORKDIR /home/OpenPLC_v2/
RUN sudo chmod +x ./build.sh
RUN printf "n\n1\n" | bash /build.sh



EXPOSE 502
EXPOSE 8080

RUN  sudo chmod +x /home/OpenPLC_v3/start_openplc.sh

CMD  nohup honeyd -d -f /app/honeyd.conf &> honeyd.log; sudo nodejs server.js

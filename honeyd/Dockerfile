FROM ubuntu:xenial

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
    automake

RUN git clone https://github.com/DataSoft/Honeyd.git
RUN cd Honeyd \
    && ./autogen.sh \
    && ./configure \
    && make \
    && make install


RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


RUN mkdir -p /app/logs
RUN touch "/app/logs/test.log"
RUN ls -alh /app
RUN ip a

ADD honeyd.conf .
# ADD Siemens-S7-1200.py .
# # Ports
# EXPOSE 502 102
ADD libsnap7.so /usr/lib/libsnap7.so
ADD s7commServer /usr/share/honeyd/s7commServer
RUN chmod 777 /usr/share/honeyd/s7commServer
# CMD ["honeyd", "-d", "-i", "eth0", "-f", "honeyd.conf", "-l", "/app/logs/test.log"]
#"-d", "-f", "honeyd.conf", "-l", "/app/logs/test.log"]


FROM ubuntu:18.04

USER root
WORKDIR /root/
RUN apt-get update && \
    apt-get -y install git python-pip autoconf bison build-essential pkg-config bison flex autoconf automake libtool make git python2.7 python-pip sqlite3 cmake sudo wget gcc make openssl libffi-dev libgdbm-dev libsqlite3-dev libssl-dev zlib1g-dev git libdnet-dev libevent-dev libdumbnet-dev libpcap-dev libpcre3-dev libedit-dev bison flex libtool automake git zlib1g-dev make
RUN pip install flask flask-login pyserial pymodbus



WORKDIR /home/

COPY Honeyd/ /home/Honeyd/
WORKDIR Honeyd

RUN mv configure.in configure.ac
RUN sudo chmod +x autogen.sh
RUN ./autogen.sh
RUN ./configure

RUN make 
RUN make install 

COPY signatures.sh /app/signatures.sh
WORKDIR /app
ADD honeyd.conf /app/honeyd.conf
RUN chmod +x signatures.sh && ./signatures.sh

WORKDIR /home/

RUN git clone https://github.com/thiagoralves/OpenPLC_v3.git
WORKDIR /home/OpenPLC_v3/
RUN ls -al install.sh
RUN sudo chmod +x install.sh
RUN sudo ./install.sh linux

RUN mkdir /home/OpenPLC_v3/scripts

COPY run.sh /home/OpenPLC_v3/
RUN sudo chmod +x run.sh
COPY PLC1.st /home/OpenPLC_v3/scripts
COPY launch.sh /home/OpenPLC_v3/launch.sh


EXPOSE 502
EXPOSE 8080


CMD  nohup honeyd -d -f /app/honeyd.conf &> honeyd.log; sh /home/OpenPLC_v3/run.sh

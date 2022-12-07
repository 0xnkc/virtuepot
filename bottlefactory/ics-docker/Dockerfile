FROM ubuntu:20.04

RUN apt-get update

RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata

RUN apt-get update \
&& apt-get install -y sudo \
&& apt-get install -y python3 \
&& apt-get install -y iputils-ping \
&& apt-get install -y net-tools \
&& apt-get install -y git \
&& apt-get -y install nano \
&& apt --yes install python3-pip \	
&& pip install pyModbusTCP \
&& apt install telnet \
&& apt-get install -y memcached \
&& apt-get install -y python3-memcache \ 
&& apt-get install -y ettercap-common \
&& apt-get install -y unzip \
&& apt-get install -y nmap \
&& apt-get install -y curl

RUN curl -OL https://raw.githubusercontent.com/nikhilkc96/virtuepot/main/bottlefactory/ics-docker/src.zip

RUN unzip src.zip

# RUN mkdir src

# COPY /src /src

WORKDIR /src




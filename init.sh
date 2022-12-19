

#!/bin/bash

cwd=$(pwd)
version=$(lsb_release -rs )

# Wrong version warning
if [ "$version" != "20.04" ] && [ "$version" != "18.04" ]
then
  printf "Warning! This installation script has only been tested on Ubuntu 20.04 LTS and 18.04 LTS and will likely not work on your Ubuntu version.\n\n"
fi

sleep 3

# Update apt
sudo apt update


# Installing necessary packages
sudo apt install -y libevent-dev libdumbnet-dev libpcap-dev libpcre3-dev libedit-dev bison flex libtool automake python3-pip python3

# Install farpd
sudo apt install -y farpd

# Get python2 pip
curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py
sudo python2 get-pip.py
rm get-pip.py

# CPPPO Correct Version 4.3.4
sudo pip2 install cpppo==4.3.4

# Installing Honeyd - just a copy and paste of the guide
cd ~
git clone https://github.com/DataSoft/Honeyd
cd Honeyd
sudo apt install -y libevent-dev libdumbnet-dev libpcap-dev libpcre3-dev libedit-dev bison flex libtool automake make zlib1g-dev
./autogen.sh
./configure
make
sudo make install




sysctl -w vm.max_map_count=262144
 
# RUN Bottle Factory 
docker-compose -f bottlefactory/docker-compose.yml up -d

# RUN Water tank Openplc
docker-compose -f watertank/docker-compose.yml up -d


# RUN SCADA LTS
docker-compose -f scada/docker-compose.yml up -d


# RUN arkime
docker-compose -f arkime/docker-compose.yml up -d

#Check the running containers 
docker ps
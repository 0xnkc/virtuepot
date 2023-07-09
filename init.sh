

#!/bin/bash

cwd=$(pwd)
version=$(lsb_release -rs )

# Wrong version warning
if [ "$version" != "22.04" ] && [ "$version" != "20.04" ] && [ "$version" != "18.04" ];
then
  printf "Warning! This installation script has only been tested on Ubuntu 20.04, 22.04 LTS and 18.04 LTS and will likely not work on your Ubuntu version.\n\n"
fi

sleep 3

# Update apt
sudo apt update


sysctl -w vm.max_map_count=262144

 
# RUN Bottle Factory 
docker-compose -f bottlefactory/docker-compose.yml up -d

# RUN Water tank Openplc
docker-compose -f watertank/docker-compose.yml up -d


# RUN SCADA LTS
docker-compose -f scada/docker-compose.yml up -d


#Run the zeek
docker-compose -f zeek/docker-compose.yml up -d


#Run the honeyd
docker-compose -f honeyd/docker-compose.yml up -d


#Check the running containers 
docker ps
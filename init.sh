

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
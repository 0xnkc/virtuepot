## Install Docker using the repository
1. Update the apt package index and install packages to allow apt to use a repository over HTTPS:

``` bash
sudo apt-get update
sudo apt-get install \
    ca-certificates \
    curl \
    gnupg \
    lsb-release
```


2. Add Docker’s official GPG key:

``` shell

sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
```

3. Use the following command to set up the repository:


``` bash 
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null 

```
4. Update the apt package index:
```bash
sudo apt-get update
Receiving a GPG error when running apt-get update?

#Your default umask may be incorrectly configured, preventing detection of the repository public key file. Try granting read permission for the Docker public key file before updating the package index:

sudo chmod a+r /etc/apt/keyrings/docker.gpg
sudo apt-get update
```

5. Install Docker Engine, containers, and Docker Compose.
``` bash 
sudo apt-get install docker-ce docker-ce-cli containerd.io docker-compose-plugin docker-compose
```
If you got permission denied while trying to connect to the Docker daemon socket

``` bash
sudo chmod 666 /var/run/docker.sock
```

6. Create the Docker network.

Docker network for icsnet 

``` bash
docker network create  --driver=bridge --subnet=192.168.0.0/24 --gateway=192.168.0.1  --opt com.docker.network.bridge.name=br_icsnet icsnet

```
Docker network for phynet 
``` bash 
docker network create  --driver=bridge --subnet=192.168.1.0/24 --gateway=192.168.1.1  --opt com.docker.network.bridge.name=br_phynet phynet
```
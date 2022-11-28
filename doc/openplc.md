## Install [OpenPLC_V2](https://github.com/thiagoralves/OpenPLC_v2) on Docker 

**http://www.openplcproject.com/**

``` bash
cd OpenPLC
docker build -t openplc .
docker network create --subnet=172.25.0.0/16 mynet
#docker pull nikhilkc96/openplc
docker run -d --rm --privileged -p 8080:8080 -p 502:502 --net mynet --ip 172.25.0.5 openplc
```

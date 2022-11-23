FROM ubuntu:20.04

RUN apt-get update

RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata

RUN apt-get update \
    && apt-get install -y sudo \
    && apt-get install -y wget  \
    && apt-get install unzip


RUN wget https://rapidscada.org/download/scada_5.8.4_mono_en.zip

RUN unzip scada_5.8.4_mono_en.zip


RUN cp -r scada/ /opt/

RUN cp  daemons/* /etc/init.d


RUN chmod -R ugo+rwx /opt/scada/ScadaWeb/config \
    && chmod -R ugo+rwx /opt/scada/ScadaWeb/log    \
    && chmod -R ugo+rwx /opt/scada/ScadaWeb/storage


RUN chmod +x /opt/scada/make_executable.sh \
    &&  /opt/scada/make_executable.sh


RUN  apt install -y  gnupg ca-certificates \
    && apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF \
    && echo "deb https://download.mono-project.com/repo/ubuntu stable-focal main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list \
    &&  sudo apt -y update


RUN apt-get install -y mono-complete \
    &&  apt-get install -y apache2


RUN  apt-get install -y libapache2-mod-mono mono-apache-server4

RUN ln -s /opt/scada/ScadaWeb /var/www/html/scada

RUN cp apache/scada.conf /etc/apache2/sites-available


RUN a2ensite scada.conf

RUN /opt/scada/svc_install.sh

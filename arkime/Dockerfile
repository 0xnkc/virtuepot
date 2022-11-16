ARG UBUNTU_VERSION=22.04
FROM ubuntu:$UBUNTU_VERSION
MAINTAINER mammo0 - https://github.com/mammo0

# Install dependencies that are needed, but not set in the arkime.deb file
RUN apt-get -qq update && \
    apt-get install -yq curl libmagic-dev wget logrotate

# Declare args
ARG ARKIME_VERSION=4.0.1
ARG UBUNTU_VERSION
ARG ARKIME_DEB_PACKAGE="arkime_"$ARKIME_VERSION"-1_amd64.deb"

# Declare envs vars for each arg
ENV ARKIME_VERSION $ARKIME_VERSION
ENV OS_HOST "opensearch"
ENV OS_PORT 9200
ENV ARKIME_INTERFACE "eth0"
ENV ARKIME_ADMIN_PASSWORD "admin"
ENV ARKIME_HOSTNAME "localhost"
ENV ARKIMEDIR "/opt/arkime"
ENV CAPTURE "off"
ENV VIEWER "on"

# Install Arkime
RUN mkdir -p /data && \
    cd /data && \
    curl -C - -O "https://s3.amazonaws.com/files.molo.ch/builds/ubuntu-"$UBUNTU_VERSION"/"$ARKIME_DEB_PACKAGE && \
    dpkg -i $ARKIME_DEB_PACKAGE || true && \
    apt-get install -yqf && \
    mv $ARKIMEDIR/etc /data/config && \
    ln -s /data/config $ARKIMEDIR/etc && \
    ln -s /data/logs $ARKIMEDIR/logs && \
    ln -s /data/pcap $ARKIMEDIR/raw
# clean up
RUN rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /var/cache/* && \
    rm /data/$ARKIME_DEB_PACKAGE

# add scripts
ADD /scripts /data/
RUN chmod 755 /data/*.sh

VOLUME ["/data/pcap", "/data/config", "/data/logs"]
EXPOSE 8005
WORKDIR $ARKIMEDIR

ENTRYPOINT ["/data/startarkime.sh"]

FROM alpine:3.12

LABEL maintainer "https://github.com/blacktop"

ARG VERSION

RUN apk add --no-cache libc6-compat curl jq

RUN \
  cd /tmp \
  && wget https://artifacts.elastic.co/downloads/beats/filebeat/filebeat-${VERSION}-linux-x86_64.tar.gz \
  && tar xzvf filebeat-${VERSION}-linux-x86_64.tar.gz \
  && mv filebeat-${VERSION}-linux-x86_64 /usr/share/filebeat \
  && mkdir /usr/share/filebeat/logs /usr/share/filebeat/data \
  && rm /tmp/*

ENV PATH $PATH:/usr/share/filebeat

COPY config /usr/share/filebeat

COPY entrypoint.sh /usr/local/bin/entrypoint
RUN chmod +x /usr/local/bin/entrypoint

WORKDIR /usr/share/filebeat

ENTRYPOINT ["entrypoint"]
CMD ["-h"]
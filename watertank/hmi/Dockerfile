FROM python:3.8-slim-buster

COPY /hmi /hmi

WORKDIR /hmi

RUN pip3 install -r requirements.txt

RUN chmod +x docker_entrypoint.sh
ENTRYPOINT [ "python" ]

CMD ["./docker_entrypoint.sh"]
FROM python:3.8-slim-buster

COPY /hmi /hmi

WORKDIR /hmi

RUN pip3 install -r requirements.txt

RUN chmod +x docker_entrypoint.sh
ENTRYPOINT [ "python" ]

CMD ["python3", "-m", "flask", "run", "--host=0.0.0.0", "--port=7000" ]
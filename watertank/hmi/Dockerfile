FROM python:3.9-slim-buster

COPY . /hmi

WORKDIR /hmi

RUN pip3 install -r requirements.txt

RUN chmod +x docker_entrypoint.sh

CMD ["gunicorn", "app:app", "-b", "0.0.0.0:5000"]

# CMD ["./docker_entrypoint.sh"]
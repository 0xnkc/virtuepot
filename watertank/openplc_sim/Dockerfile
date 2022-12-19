FROM python:3.9-slim-buster
ENV PYTHONUNBUFFERED=1
COPY . /app
WORKDIR /app

RUN pip3 install -r requirements.txt

CMD [ "python3", "tcp_modbus.py"]

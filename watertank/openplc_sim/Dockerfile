FROM python:3.8-slim-buster
ENV PYTHONUNBUFFERED=1
COPY . /app
WORKDIR /app

RUN pip install -r requirements.txt

CMD [ "python3", "tcp_modbus.py"]

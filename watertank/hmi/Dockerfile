FROM python:3.9-slim-buster

COPY . /hmi

WORKDIR /hmi

RUN pip3 install -r requirements.txt

RUN chmod +x docker_entrypoint.sh

#CMD [ "python3", "-m" , "flask", "run", "--host=0.0.0.0"]

# ENTRYPOINT ["/bin/bash", "-c", "python3 tcp_modbus.py & python3 -m flask run --host=0.0.0.0 "]
ENV FLASK_APP=app.py
EXPOSE 5000
CMD [ "flask", "run","--host","0.0.0.0","--port","5000"]
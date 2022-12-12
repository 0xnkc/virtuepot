#!/bin/bash

set -e

exec python3 tcp_modbus.py &
exec python3 -m flask run --host=0.0.0.0 --port=7000 
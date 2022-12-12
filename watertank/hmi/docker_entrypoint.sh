#!/bin/bash

set -e

exec python3 tcp_modbus.py &
exec flask run --host=0.0.0.0 --port=8080 
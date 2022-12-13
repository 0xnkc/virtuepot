#!/bin/bash

set -e

exec python3 tcp_modbus.py &
exec gunicorn app:app -b 0.0.0.0:5000
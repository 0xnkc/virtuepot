#!/bin/sh

# Launch the first application

./home/OpenPLC_v3/run.sh &

# Launch the second application
#./home/OpenPLC_v3/run.sh &

# Keep the container running
tail -f /dev/null

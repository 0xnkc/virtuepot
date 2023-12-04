#!/bin/bash
# add @hourly path/restart_docker_containers.sh to the cortab by cmd $ crontab -e
# Get a list of all Docker containers
containers=$(docker ps -a -q)

# Iterate through each container
for container in $containers
do
    # Check if the container is stopped
    if [ "$(docker inspect -f '{{.State.Status}}' $container)" = "exited" ]; then
        # Restart the container
        docker restart $container
        echo "Container $container restarted."
    fi
done

echo "Finished checking and restarting Docker containers."

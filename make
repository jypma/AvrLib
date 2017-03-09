#!/bin/bash
#docker-compose build
docker run -ti --rm -v $(pwd):/tmp/hx -w /tmp/hx -v /etc/group:/etc/group:ro -v /etc/passwd:/etc/passwd:ro --user=$UID jypma/avr-gcc:5.3 make "$@"

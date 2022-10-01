#!/bin/bash

#Create folders to store data, if necessary
if [ ! -d ".logs" ]; then
    mkdir .logs
fi

mv server.log "./.logs/$(date '+%Y%m%d%H%M').log"
touch server.log

clear
./server
tail -f server.log
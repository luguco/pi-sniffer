#!/bin/bash

# Stop on any error
set -e

git pull

gcc `pkg-config --cflags glib-2.0 gio-2.0` -Wall -Wextra -o scan scan.c mqtt.c mqtt_pal.c -lm `pkg-config --libs glib-2.0 gio-2.0`

chmod a+x ./scan

# run immediately
# sudo ./scan 192.168.0.120 1883

# update running service

if [ -e /lib/systemd/system/pi-sniffer.service ]
then
    echo stopping and restarting service
    sudo systemctl stop pi-sniffer.service
#    sudo systemctl start pi-sniffer.service
else
    echo creating service
    echo installing service
    sudo cp pi-sniffer.service /lib/systemd/system/
    sudo chmod 644 /lib/systemd/system/pi-sniffer.service
    sudo systemctl daemon-reload
    sudo systemctl enable pi-sniffer.service
    echo starting service
    sudo systemctl start pi-sniffer.service
fi

echo now tailing log, ctrl-c to stop but leave service running
sudo journalctl -u pi-sniffer.service -f

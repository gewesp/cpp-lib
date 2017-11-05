#! /bin/bash

#
# Connects to OGN and monitors the data stream for the given IDs.
# If found, outputs an alert on stderr and also logs to syslog.
#
# Reconnects after 10 seconds if the connection drops.
#
# Requires: nc (netcat), install on Ubuntu: apt-get install netcat
#

TAG=OGN-SEARCH

# Set IDs to search for in this variable
IDs='FLR000002|DEE5B3|DD4E39|DD4E3C|DD4F01|DD4F4D|DDB04B'

HOST=aprs.glidernet.org
PORT=10152

while true ; do
  echo 'user 0 pass -1 vers ogns/1.0.0' | \
      nc -q -1 $HOST $PORT              | \
      egrep "$IDs"                      | \
      logger -s -t "$TAG":ALERT
  logger -s -t $TAG "Connection dropped; reconnecting after 10 seconds" 
  sleep 10
done

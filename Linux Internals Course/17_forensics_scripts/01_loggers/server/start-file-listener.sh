#!/bin/bash

# Simple script to start a new file listener.
# When a filename is sent to port 5555 a transfer on 5556 is expected to follow.

usage () {
	echo "usage: $0 <case name>"
	echo "Simple script to start a file listener"
	exit 1
}

# did you specify a file?
if [ $# -lt 1 ] ; then
   usage
fi

while true
do
   filename=$(nc -l 5555)
   nc -l 5556 > $1/$(basename $filename)
done

#!/bin/bash

# Simple script to shut down listeners.

echo "Shutting down listeners at $(date) at user request" | nc localhost 4444
killall start-case.sh
killall start-file-listener.sh
killall nc


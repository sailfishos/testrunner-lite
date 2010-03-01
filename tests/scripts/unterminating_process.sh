#!/bin/sh

trap '' SIGTERM

echo "echoing to stdout"
echo "echoing to stderr" 1>&2
sleep 120

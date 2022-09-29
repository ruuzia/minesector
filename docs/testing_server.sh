#!/usr/bin/sh

killall ngrok
killall python3

exec python3 -m http.server 3468 1> /dev/null 2> /dev/null 3> /dev/null &
exec ngrok http 3468

#!/bin/sh
if [ -e server.pid ]; then
    if ( kill -0 $(cat ts3server.pid) 2> /dev/null ); then
        echo "The server is already running"
        exit 1
    else
        echo "server.pid found, but no server running"
        rm server.pid
    fi
fi
echo "Starting the SAMP server"
if [ -e samp03svr ]; then
    if [ ! -x samp03svr ]; then
        echo "samp03svr is not executable, trying to set it"
        chmod u+x "samp03svr"
    fi
    if [ -x samp03svr ]; then
        "./samp03svr" > /dev/null &
        echo $! > server.pid
        echo "SAMP server started"
    else
        echo "$samp03svr is not exectuable"
    fi
else
    echo "Could not find SAMP server"
    exit 5
fi

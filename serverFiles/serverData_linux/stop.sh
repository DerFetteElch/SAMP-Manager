#!/bin/sh
if [ -e server.pid ]; then
    echo -n "Stopping the SAMP server"
	if ( kill -TERM $(cat server.pid) 2> /dev/null ); then
        a=1
        while [ "$a" -le 300 ]; do
            if ( kill -0 $(cat server.pid) 2> /dev/null ); then
                echo -n "."
                sleep 1
            else
                break
            fi
            a=$((++a))
        done
    fi
    if ( kill -0 $(cat server.pid) 2> /dev/null ); then         
        kill -KILL $(cat server.pid)
    else
        echo "done"
    fi
    rm server.pid
else
    echo "server.pid is missing"
    exit 7
fi

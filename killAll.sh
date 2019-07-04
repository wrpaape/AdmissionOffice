#!/bin/bash

for proc in Student Department Admission
do
    pids=$(pidof $proc)
    if [ -n "$pids" ]
    then
        kill $pids
    fi
done

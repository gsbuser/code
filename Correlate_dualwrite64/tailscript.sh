#!/bin/sh -x
for (( i=0;i<25000;i++)) ; do
fn=`ls -t LOG*|head -1`
tail -f $fn &
sleep 200
killall tail
done


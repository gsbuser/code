#!/bin/sh

for ((i=3; i<10; i++)) ; do
let "j=$i % 3" 
case "$j" in
    
    0)
    echo "No remainder!"
    ;;
    1)
    echo "1 left!"
    ;;
    2)
    echo "2 left..."
    ;;
esac

done
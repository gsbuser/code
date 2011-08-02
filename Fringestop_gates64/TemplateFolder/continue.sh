#!/bin/sh
for ((i=33 ; i<51 ; i++)) ; do
    ( ssh node$i killall -CONT fringestop.x & );
done

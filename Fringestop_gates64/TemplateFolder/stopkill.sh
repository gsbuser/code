#!/bin/sh
for ((i=33 ; i<49 ; i++)) ; do
ssh node$i killall fringestop.x ;
done

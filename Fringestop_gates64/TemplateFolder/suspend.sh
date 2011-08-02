#!/bin/sh
for ((i=33 ; i<51 ; i++)) ; do
    (ssh node$i killall -STOP fringestop.x &);
done

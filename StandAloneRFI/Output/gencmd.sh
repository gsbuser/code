#!/bin/bash
./azpointing.x $* > coord1.dat
cat << END1
*********************************************************************
* 30 antennas Azimuth position for Mr. Ue-Li-Pen's RFI experiment.
* Set all antennas to the following set of az and el coordinates.
* These are the coordinates for file $1
*********************************************************************

cmode 1
lnkndasq
subar 4

suba 4
shsub 4
stabct


discmdmon

END1
index=( 7 6 5 1 3 19 20 24 4 12 9 2 11 10 8 13 14 15 16 25 17 18 21 22 23 26 27 28 29 30 )
for (( i=0;i<30;i++)) ; do
az=`head -$((i+2)) coord1.dat | tail -1| gawk '{print $3}'`
ant=`head -$((i+2)) coord1.dat | tail -1| gawk '{print $2}'`
echo "* $ant"
echo ante 1 ${index[$i]}
echo "cp 0;defs 0;suba 0"
echo "amv("$az"d,18d)"
echo sleep 2
echo
done

cat << END2

* Configuring all antennas to suba 4.
allant
cp 0;defs 4;suba 4
sndsacant

enacmdmon

/bell
end

END2

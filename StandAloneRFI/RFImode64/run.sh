#!/bin/sh

if [[ $# != 1 ]]; then echo "Usage: `basename $0` basename"; exit -1; fi

#basename=dec.20.08.b0823.4
basename=$1

svdrfi=../../SVDRFI/$basename/$basename.svdrfi.node%X
#svdrfi=/mnt/raid-project/gmrt/paciga/Finals/Feb.25.10/PSR-B0823.11.R80/SVDRFI/feb.25.10.b0823.11.svdrfi.node%X
outpref=output/$basename

startmode=1
endmode=10

for mode in `seq $startmode $endmode`
do
  omode=$(printf '%02d' $mode)
  echo "==================== $omode ===================="
  lag=$outpref.lag.$omode.dat
  ./rfimode.x $svdrfi $outpref $mode | tee $outpref.$omode.out
  ./rfiposlag.x $lag $outpref $mode | tee -a $outpref.$omode.out

#  cp $outpref.lagnoshift.$omode.dat lagnoshift.dat
#  ./rfipos.x $lag $outpref $mode | tee -a $outpref.$omode.out
  mv chimap.pgm $outpref.chimap.$omode.pgm

done

#grep -a ' @' $outpref.*.out | awk '{print $2,$3,$4,"'$basename'"}' > $outpref.coords.dat
grep -a ' @' $outpref.*.out | awk '{print $2,$3,$4,$1}' > $outpref.coords.dat
awk -f kml.awk $outpref.coords.dat > $outpref.kml

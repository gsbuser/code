#!/bin/sh

if [ $# != 1 ]; then echo "Usage: `basename $0` basename"; exit -1; fi

basename=$1

svdtvec=$basename.svdtvec.dat
outpref=$basename

od -t f8 -j4 -w8 -Ad $svdtvec > $outpref.svdtvec.txt
nlines=$(echo "( $(wc -l $outpref.svdtvec.txt | awk '{print $1}') -1 ) / 50" | bc)
split -l $nlines $outpref.svdtvec.txt

for file in x??
  do
  newfile=${file/x/tvec.}  # changes only first instance of x
  base26=${newfile##*.}
  base10=$(../RFImode/base26to10.awk -v num=$base26)
  mode=$(printf "%02d" $(echo "${base10} + 1 " | bc))
  newfile=${newfile/$base26/$mode}
  echo $outpref.$newfile
  awk "{print \$1-$nlines*8*$base10,\$2}" $file > $outpref.$newfile
  rm $file
done
rm $outpref.$newfile # the last file created is empty


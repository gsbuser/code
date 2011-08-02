#!/bin/bash -x

if [[ -z $1 ]]; then
  echo "USAGE: $0 MONTH(Mmm) DAY(dd)"
  exit
fi

fn="${1}${2}nocal"
PI=3.14159265
echo 1> "${fn}.plt"

while read i; do
  line="$i"
  #echo "processing $line"
  ant=${line%%[ ,]*}
  ant=${ant# }
  ant=${ant% }
 
  m=${line##*m=(}
  um=${m##* +/- }
  m=${m%% +/- *}
  um=${um%%)}
  m=${m# }
  m=${m% }
  um=${um# }
  um=${um% }

  b=${line##*b=(}
  ub=${b##* +/- }
  b=${b%% +/- *}
  ub=${ub%%)}
  b=${b# }
  b=${b% }
  ub=${ub# }
  ub=${ub% }
  echo "$ant  $m  $b  $b  $um  $ub"
  if ! [[ $ant -eq $ant ]]; then
    echo "$ant  $m  $b  $um  $ub"
    echo "BAD LINE (ant): $line"
    continue
  fi
  m=$(echo $m| bc -l)
  if ! [[  $m =~ ^-?[0-9]*([.][0-9]+)?$ ]]; then
    echo "$ant  $m  $b  $um  $ub"
    echo "BAD LINE (m): $line"
    continue
  fi
  b=$(echo $b| bc -l)
  if ! [[  $b =~ ^-?[0-9]*([.][0-9]+)?$ ]]; then
    echo "$ant  $m  $b  $um  $ub"
    echo "BAD LINE (b): $line"
    continue
  fi
  um=$(echo $um| bc -l)
  if ! [[  $um =~ ^-?[0-9]*([.][0-9]+)?$ ]]; then
    echo "$ant  $m  $b  $um  $ub"
    echo "BAD LINE (um): $line"
    continue
  fi
  ub=$(echo $ub| bc -l)
  if ! [[  $ub =~ ^-?[0-9]*([.][0-9]+)?$ ]]; then
    echo "$ant  $m  $b  $um  $ub"
    echo "BAD LINE (ub): $line"
    continue
  fi
  #mod yint
  if [[ "$(echo "($b) < 0" | bc -l)" == "1" ]]; then
    echo "adding pi"
    yint=$(echo $b + $PI | bc -l)
  else
    yint=$b
  fi
  echo "$ant  $m  $b  $yint  $um  $ub" 1>> "${fn}.plt"
  echo "$ant  $m  $b  $yint  $um  $ub"
done < $1
echo "Saved as ${fn}.plt"
# 2.21365927
echo "Getting timestamp from gsbm2:/mnt/a/gsbuser/EoR/Tapes/scratch/paciga/PolCal/${1}.${2}.07/Fimage"
maxrdiff=$PI
while read i; do
  t=${i%% *}; r=${i##* };
  echo '$t  $r';
done < "/mnt/a/gsbuser/EoR/Tapes/scratch/paciga/PolCal/${1}.${2}.07/Fimage/times256.dat"
exit


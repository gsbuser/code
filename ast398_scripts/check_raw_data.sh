#2011.06.27
#Finds teh sum of data usage taken up by files starting with b2045-16_* on each of the nodes 33-48 in directories EoR[a/b/c/d]
#Want only the *nodeX and *nodeX.norm files, not the *nodeX.fs0 files
#Run from one of the nodes 33-48
#takes in the three character representation for a month

mon=$1
#EoRa
EoRa=0
for num in `ls -l /mnt/EoRa/gsbuser/EoR/b2045-16_$mon* |awk '{print $5}'`
  do EoRa=$(echo "$num + $EoRa" | bc -l)
  #echo "EoRa: $EoRa"
done
echo "EoRa: $EoRa"

fs0a=0
for num in `ls -l /mnt/EoRa/gsbuser/EoR/b2045-16_$mon*.fs0 |awk '{print $5}'`
  do fs0a=$(echo "$num + $fs0a" | bc -l)
  #echo "fs0a: $fs0a"
done

echo "fs0a: $fs0a"
EoRa=$(echo "$EoRa-$fs0a" | bc -l)
echo "Final EoRa: $EoRa"

#EoRb
EoRb=0
for num in `ls -l /mnt/EoRb/gsbuser/EoR/b2045-16_$mon* |awk '{print $5}'`
  do EoRb=$(echo "$num + $EoRb" | bc -l)
  #echo "EoRb: $EoRb"
done
echo "EoRb: $EoRb"

fs0b=0
for num in `ls -l /mnt/EoRb/gsbuser/EoR/b2045-16_$mon*.fs0 |awk '{print $5}'`
  do fs0b=$(echo "$num + $fs0b" | bc -l)
  #echo "fs0b: $fs0b"
done
echo "fs0b: $fs0b"
EoRb=$(echo "$EoRb-$fs0b" | bc -l)
echo "Final EoRb: $EoRb"

#EoRc
EoRc=0
for num in `ls -l /mnt/EoRc/gsbuser/EoR/b2045-16_$mon* |awk '{print $5}'`
  do EoRc=$(echo "$num + $EoRc" | bc -l)
  #echo "EoRc: $EoRc"
done
echo "EoRc: $EoRc"

fs0c=0
for num in `ls -l /mnt/EoRc/gsbuser/EoR/b2045-16_$mon*.fs0 |awk '{print $5}'`
  do fs0c=$(echo "$num + $fs0c" | bc -l)
  #echo "fs0c: $fs0c"
done
echo "fs0c: $fs0c"

EoRc=$(echo "$EoRc-$fs0c" | bc -l)
echo "Final EoRc: $EoRc"

#EoRd
EoRd=0
for num in `ls -l /mnt/EoRd/gsbuser/EoR/b2045-16_$mon* |awk '{print $5}'`
  do EoRd=$(echo "$num + $EoRd" | bc -l)
  #echo "EoRd: $EoRd"
done
echo "EoRd: $EoRd"

fs0d=0
for num in `ls -l /mnt/EoRd/gsbuser/EoR/b2045-16_$mon*.fs0 |awk '{print $5}'`
  do fs0d=$(echo "$num + $fs0d" | bc -l)
  #echo "fs0d: $fs0d"
done

echo "fs0d: $fs0d"
EoRd=$(echo "$EoRd-$fs0d" | bc -l)
echo "Final EoRd: $EoRd"

total=$(echo "($EoRa+$EoRb+$EoRb+$EoRd)*16" | bc -l)
echo "total: $total"
total=$(echo "$total/(1000000000000)" | bc -l)
echo "total TB: $total"
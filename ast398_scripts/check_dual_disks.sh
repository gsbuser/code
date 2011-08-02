#Hans Nguyen 2011/06/17
#Outputs the least available space over all nodes in each directory EoR[abcd]
#use from gsbm1
#checking EoRa
smalla='9999999999999999999999999999'
smallb='9999999999999999999999999999'
smallc='9999999999999999999999999999'
smalld='9999999999999999999999999999'

for ((i=33; i<49; i++));
  do
  temp=$(ssh node$i df -h | grep EoRa | awk '{print $4}' | sed 's/G//'); #echo $temp;
  if [[ $temp -lt $smalla ]]
      then
      smalla=$temp     
  fi
done
echo "EoRa: $smalla G free"

#checking EoRb
for ((i=33; i<49; i++));
  do
  temp=$(ssh node$i df -h | grep EoRb | awk '{print $4}' | sed 's/G//'); #echo $temp;
  if [[ $temp -lt $smallb ]]
      then
      smallb=$temp     
  fi
done
echo "EoRb: $smallb G free"

#checking EoRc
for ((i=33; i<49; i++));
  do
  temp=$(ssh node$i df -h | grep EoRc | awk '{print $4}' | sed 's/G//'); #echo "=== $i ==="; echo $temp;
  if [[ $temp -lt $smallc ]]
      then
      #echo "Changing $smallc to $temp"
      smallc=$temp 
      #echo "smallc $smallc"
  fi
done
echo "EoRc: $smallc G free"

#checking EoRd
for ((i=33; i<49; i++));
  do
  temp=$(ssh node$i df -h | grep EoRd | awk '{print $4}' | sed 's/G//'); #echo $temp;
  if [[ $temp -lt $smalld ]]
      then
      smalld=$temp     
  fi
done
echo "EoRd: $smalld G free"


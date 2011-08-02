#2011/20/06
#Iterates through nodes 0-15 to decode files onto 33-48 respectively
echo "make sure you are on gsbm1"

if [[ -z $1 ]]; then
	echo "give the file name, probably rfi_mmmdd_run"
	exit
fi

file=$1 #enter filename prior to the '.nodeX'
disk=EoRa

for ((i=0; i<16; i++));
  do
  (echo ssh node$((i+33)) /mnt/code/gsbuser/EoR/Decode2/decoderaw-1byte.x /mnt/${disk}/gsbuser/EoR/$file.node$i &)
done




#2011/20/06
#Iterates through nodes 0-15 to decode files onto 33-48 respectively
echo "make sure you are on gsbm1"

if [[ -z $1 ]]; then
	echo "rfi_{mmmdd} {startrun} {lastrun] {disk}"
	exit
fi

startrun=$2
lastrun=$3
disk=$4
for ((j=$startrun; j<=$lastrun; j++)); do
	file=${1}_${j}
	echo $file
	for ((i=0; i<16; i++));	do
  		(ssh node$((i+33)) /mnt/code/gsbuser/EoR/Decode2/decoderaw-1byte.x /mnt/${disk}/gsbuser/EoR/$file.node$i &)
	done &> LOG_$file
done




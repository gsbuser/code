#!/bin/bash

if [[ -z $1 ]]; then
	echo "USAGE: $0 KMLMAP FOLDER EXEMPT DESCRIPTION STYLE"
	exit 1
fi

if [[ "$#" != "5" ]]; then
	echo "Not enough args"
	exit 1
fi

STYLE="$5"

RFI_OUT="/mnt/EoR/gsbuser/EoR/RFI_Out"
FOLDER=${RFI_OUT}/${2%/*}
echo "In $FOLDER"
for i in $(ls ${FOLDER}/*.out); do
	cm=$(grep -a modenum $i)
	cm=$(echo ${cm#*=})
	name=${2%/*}.${cm}
#	name=$(echo ${name%.out})
	B=0
	for m in $3; do
		#echo "Testing $cm against $m"
		if [[ "$cm" == "$m" ]]; then
			echo "Skipping chimap $cm"
			B=1
			break
		fi
	done
	if [[ "$B" == "1" ]]; then continue; fi;
	line=$(grep -a " \@" $i)
	N=${line##*\@}
	E=$(echo ${N##*,})
	N=$(echo ${N%%,*})
	if [[ "!${N}" == "0" ]]; then
		echo help
	fi
	echo "$RFI_OUT/kmlmapper.out \"$1\" \"$name\" \"$4\" $N $E \"$STYLE\""
	$RFI_OUT/kmlmapper.out "$1" "$name" "$4" $N $E "$STYLE"
		
done

exit


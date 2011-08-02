#!/bin/bash

# This script reads the delay table for the standard GSB correlator and
# converts it to microseconds with the format and antenna order used for the EoR correlator
# This should probably be run every time the GSB delays change

# Greg Paciga, 2011-06-24


GSBdelayfile=/mnt/code/gsbuser/SYS_FILES/antsys.hdr

speed_of_light=299.792458

#date=$(date -%F)
#localGSBcopy=/mnt/code/gsbuser/EoR/Correlate_dualwrite64/antsys.hdr.$date
#cp $GSBdelayfile $localGSBcopy


# column 8 = L delay in metres
grep '^ANT' $GSBdelayfile | grep -v 'ANT3[01]' | grep '= C' | awk '{print $8/'$speed_of_light'}'
grep '^ANT' $GSBdelayfile | grep -v 'ANT3[01]' | grep '= W' | awk '{print $8/'$speed_of_light'}'
grep '^ANT' $GSBdelayfile | grep -v 'ANT3[01]' | grep '= E' | awk '{print $8/'$speed_of_light'}'
grep '^ANT' $GSBdelayfile | grep -v 'ANT3[01]' | grep '= S' | awk '{print $8/'$speed_of_light'}'
echo 0.0 # for the two empty channels
echo 0.0

# column 7 = R delay in metres
grep '^ANT' $GSBdelayfile | grep -v 'ANT3[01]' | grep '= C' | awk '{print $7/'$speed_of_light'}'
grep '^ANT' $GSBdelayfile | grep -v 'ANT3[01]' | grep '= W' | awk '{print $7/'$speed_of_light'}'
grep '^ANT' $GSBdelayfile | grep -v 'ANT3[01]' | grep '= E' | awk '{print $7/'$speed_of_light'}'
grep '^ANT' $GSBdelayfile | grep -v 'ANT3[01]' | grep '= S' | awk '{print $7/'$speed_of_light'}'
echo 0.0 # for the two empty channels
echo 0.0


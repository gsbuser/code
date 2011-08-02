#!/bin/sh

# Set to null if you only want to see what will be done
doMount=1

# Check whether we want to actually do the mounts or not
while getopts ":v" Option
  do
  case $Option in
      v ) doMount=
	  ;;
  esac
done

pref=node
#pref=192.168.14.
#pref=192.168.15.


function checkMount {
    node=$1
    disk=$2
    partner=$3
    # Check the mount record on the node and look for disk
    if mount=$(ssh $node mount | grep "on $disk type"); then
	# If found, print that we found it, and warn if the mount point is not as we expected
	echo "$node: $disk is mounted"
	if [[ ( "${partner//$pref/node}" != "${mount%% *}" ) && ( "${partner//$pref/192.168.14.}" != "${mount%% *}" ) && ( "${partner//$pref/192.168.15.}" != "${mount%% *}" ) ]]; 
	    then echo "$node: --> $disk MOUNTED TO ${mount%% *} BUT WAS EXPECTING ${partner}"; fi
    else
	# If not found, suggest the command to mount it, and carry it out if requested
	echo "$node: $disk is not mounted"
	cmd1="ssh root@$node mkdir -p $disk"
	cmd2="ssh root@$node mount $partner $disk"
	echo "$node: --> $cmd1; $cmd2"
	if [ $doMount ]; then $cmd1; $cmd2; fi
    fi
}


# Loop through all the quad core nodes
for q in `seq 17 32`
  do
  
  # Set the names of the quad core nodes and their partner dual core
  quad=$pref$q
  dual=$pref$(( q + 16 ))

  # Loop through the four disks
  for m in a b c d
  do
    
    # Set disk names analogous to the fringestop script
    DISKIN=$m
    DISKIN_dual=$m
    DISKOUT=d$m

    # Check if the dual disk is mounted locally
    disk=/mnt/$DISKIN_dual
    partner=/dev/sd${DISKIN_dual}1
    checkMount $dual $disk $partner
    
    # Check if the local disk is mounted
    disk=/mnt/$DISKIN
    partner=/dev/sd${DISKIN}1
    checkMount $quad $disk $partner

    # Check if the partner dual core disk is mounted
    disk=/mnt/$DISKOUT
    partner=$dual:/mnt/$DISKIN_dual
    checkMount $quad $disk $partner
  done

done


# Check that both test1 and test2 raid0 disks are mounted on node17
quad=${pref}17
dual=${pref}$(( 17 + 16 ))
for t in test1 test2
  do
  DISKTB=$t
  disk=/mnt/$DISKTB
  # Two possible naming conventions of the /mnt/raid0 disks
  if [[ "$DISKTB" == "ta" ]]; then testn=test1; fi
  if [[ "$DISKTB" == "tb" ]]; then testn=test2; fi
  if [[ "$DISKTB" == "test1" ]]; then testn=test1; fi
  if [[ "$DISKTB" == "test2" ]]; then testn=test2; fi
  partner=$testn:/mnt/raid0
  checkMount $quad $disk $partner
done

DISKTB=WD
disk=/mnt/$DISKTB
partner=test1:/mnt/WD
checkMount $quad $disk $partner

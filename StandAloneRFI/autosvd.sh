#!/bin/sh +x

echo "make sure you are on gsbm1"

if [[ -z $1 ]]; then
	echo "USAGE: $0 target(rfi or pulsar name) month(mmm) date(dd) year(yy) startrun lastrun disk"
#	echo "BE CAREFUL! runs them all simultaneously"
	exit
fi

# Pulled from pipeline.sh but only does SVD
# Requires .fs0 files from fringestop (decoderaw-1byte) on the dual nodes
# Update: We are switching node32 and node33 for gsbm2 and gsbm3. (did we switch back?) 
# 	Original in pipeline.old

target="$1" #pulsar name, if SVDing EoR data
rawtarget=${target}

month=$2
date=$3
year=$4
startrun=$5
lastrun=$6

disk=$7



RFImode=RFImode64
SVDRFI=SVDRFI64

# location of .fs0 files on each node (the result of decoding them data)
DATADIR=/mnt/${disk}/gsbuser/EoR
tmp=${DATADIR}/SVDRFI	#where files are put on nodes

# Make sure OUTDIR is visible to all nodes or mount the OUTDIR on all nodes. Move the svd files to gsbm3:/mnt/EoR/gsbuser/EoR/RFI_Out/
PROGDIR=/mnt/code/gsbuser/EoR/StandAloneRFI
#PROGDIR=${0%/*}
OUTMACHINE=gsbm3 # machine to copy SVD files to, where OUTDIR is located

node0=33
hostfile=$PROGDIR/dual_nodes

mpirun="time /opt/openmpi-1.2.2/bin/mpirun  -np 16 -bynode -hostfile $hostfile "



# --- EXECUTION ---
date
echo running on `hostname`
for ((run=$startrun; run<=$lastrun; run++)); do
#	export run
        basename=$month.$date.$year.$target.$run	# Naming convention
	OUTDIR=/mnt/EoR/gsbuser/EoR/RFI_Out/${basename}/
	rawbasename=${rawtarget}_${month}${date}_${run}

	for ((i=1; i<=1; i++)); do #hacky (lazy) way to get a log file for each run
#====================================================================
#=== Put functions in here

# Define some useful functions
function warn {
    # Print a warning but keep the program running
    echo -ne "Warning: "
    echo "$*"
}

function die {
    # Print an error and kill the script
    echo -ne "Error: "
    echo "$*"
    exit 1
}

function isNumber {
    # Let awk decide if this is a number and send a return value accordingly
    # Recommended usage: isNumber $VAR || die "Not a number"
    value=$1
# Better method for integers use 'if [[ $value -eq $value ]]; then we have an integer'
# Better method for floats use 'if [[ $value =~ ^-*[0-9]+([.][0-9]+)?$ ]]; then we have a float'
    echo $value | awk '{if ($1+0 != $1) exit 1; else exit 0;}'
    return $?
}

function printUsage {
    echo "---- DISK USAGE ---------------------------------------------------------------"
    df -h /
    du -sh $tmp
    find $tmp -type f -exec ls -l '{}' \; | awk '{printf("%12s  %s\n",$5,$9)}'
    echo "-------------------------------------------------------------------------------"
}


export OMP_NUM_THREADS=2
#export MKL_SERIAL=NO
export KMP_STACKSIZE=2164608

# --- STEP 1 --- File Setup ----------
Step1() {
	echo --- Starting Step 1 - File Setup ----------------------------; date

	$mpirun mkdir -p $tmp
	ssh $OUTMACHINE mkdir -p $OUTDIR
	
	# Hacky way to get the nodes into an array
	hosts=$(cat $hostfile)
	hostarray=( )
	for host in ${hosts[@]}
	  do
	  hostarray=( ${hostarray[@]} $host )
	done

	# For each node, ssh in and make symlinks with hex names that SVD can understand
	# Ideally, make SVD understand base 10
	for n in `seq 0 $(( ${#hostarray[@]} - 1 ))`
	  do
	  echo "Linking node$n (${hostarray[$n]})"
	  X=$(printf "%X" $n)
	  ssh ${hostarray[$n]} ln -s $DATADIR/$rawbasename.node$n.fs0 $tmp/$basename.node$X.fs0
	done


	#NT=`wc -l $PROGDIR/times.dat`
	#NT="${NT%% *}"
	#isNumber $NToverride && NT=$NToverride

	#NT=3949
	NT=1000
	#NT=$6
	


	NT0=$(( $NT - ( $NT % 256 ) ))
	NTA=$(( $NT0 / 256 ))
	NT05=$(( $NT0 + 5 ))
	I="-i"
        #NT0=300
	#NT0=260 #changed as of 2011.07.15, attempt to fix july 14th scan 2
	
	sed $I "s/nt=[0-9]*/nt=$NT0/g" ${SVDRFI}/svd_model.f90 #| grep nt= 
	sed $I "s/nt=[0-9]*/nt=$NT0/g" ${SVDRFI}/svd_uvec.f90 #| grep nt= 
	sed $I "s/nt=[0-9]*/nt=$NT0/g" ${SVDRFI}/svd_vvec.f90 #| grep nt= 
	cd ${SVDRFI}; make; cd $PROGDIR


	echo --- Done with Step 1 - File Setup ---------------------------; date
}

# --- STEP 3.2.3 --- SVDRFI ----------
Step323() {
	cd $PROGDIR/${SVDRFI}
	echo --- Starting Step 3.2.3 - SVDRFI I1 -------------------------; date

	export KMP_STACKSIZE=2164608
 	date
 	echo ===== Starting svd_uvec ===========================================
# 	$mpirun ./svd_uvec.x $tmp/$basename.node%X.fs0 \
# 	    $tmp/$basename.svdtvec.dat \
# 	    $tmp/$basename.eval.dat
 	echo ===== Done svd_uvec ===============================================
 	date
 	echo ===== Starting svd_vvec ===========================================
# 	$mpirun ./svd_vvec.x $tmp/$basename.node%X.fs0 \
# 	    $tmp/$basename.svdtvec.dat \
# 	    $tmp/$basename.svdrfi.node%X   
 	echo ===== Done svd_vvec ===============================================
 	date
	#echo ===== Starting svd_model ==========================================
#	$mpirun ./svd_model.x $OUTDIR/$basename.svdtvec.dat \
#	    $OUTDIR/$basename.svdrfi.node%X \
#	    $OUTDIR/$basename.corrout.dat.node%X
	#echo ===== Done svd_model ==============================================
	date
	
	ssh $OUTMACHINE "scp 192.168.16.33:$tmp/$basename.svdtvec.dat $OUTDIR/"
	ssh $OUTMACHINE "scp 192.168.16.33:$tmp/$basename.eval.dat $OUTDIR/"
	for node in `seq 33 48`
	do
	  ssh $OUTMACHINE "scp 192.168.16.${node}:$tmp/$basename.svdrfi.node'*' $OUTDIR/"
	  echo "copied $tmp/$basename.svdrfi.node'*'"
	done

	echo --- Done with Step 3.2.3 - SVDRFI I1 ------------------------; date
}




#====================================================================

		echo "SVDing run $run at $(date)"
		Step1    # File Setup
		Step323  # SVDRFI
	done | tee LOG_${target}_${month}${date}_${run}_copy #does not save the error output
done

date

echo "We are Done"
exit

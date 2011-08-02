#!/bin/sh +x

# Run SVD and make RFI maps 

# Adapted from similar program suite on NCRA HP cluster
# which was itself adapted from the CITA version

# Requires .fs0 files from fringestop on the dual nodes

#target=b2045
#rawtarget=${target}+47
target=rfi
rawtarget=${target}

month=jun
date=21
year=11
run=1
disk=EoRa


# in theory, if everything works as it should, you shouldn't need to edit anything below here 


DATADIR=/mnt/${disk}/gsbuser/EoR
tmp=${DATADIR}/SVDRFI #where files are put on nodes
basename=$month.$date.$year.$target.$run                    # Naming convention
rawbasename=${rawtarget}_${month}${date}_${run}

PROGDIR=/mnt/code/gsbuser/EoR/StandAloneRFI
OUTDIR=$PROGDIR/Output/${basename}

node0=33
hostfile=$PROGDIR/dual_nodes50

#mpirun="mpirun -np 16 -hostfile $hostfile"
mpirun="time /opt/openmpi-1.2.2/bin/mpirun  -np 16 -bynode -hostfile $hostfile --mca btl_tcp_if_include eth2,eth3"

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
	mkdir -p $OUTDIR
	
	# Hacky way to get the nodes into an array
	hosts=$(cat dual_nodes)
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

	NT0=$(( $NT - ( $NT % 256 ) ))
	NTA=$(( $NT0 / 256 ))
	NT05=$(( $NT0 + 5 ))
	I="-i"

	sed $I "s/nt=[0-9]*/nt=$NT0/g" SVDRFI/svd_model.f90 #| grep nt= 
	sed $I "s/nt=[0-9]*/nt=$NT0/g" SVDRFI/svd_uvec.f90 #| grep nt= 
	sed $I "s/nt=[0-9]*/nt=$NT0/g" SVDRFI/svd_vvec.f90 #| grep nt= 
	cd SVDRFI; make; cd $PROGDIR


	echo --- Done with Step 1 - File Setup ---------------------------; date
}

# --- STEP 3.2.0 --- Sigmacut_debias -----
Step320() {
    # Does the work of Debias, Sigmacut, and Applymask!
    cd $PROGDIR/Sigmacut_debias
    echo --- Starting Step 3.1.2 - Sigmacut Debias -----------------------; date
    make clean; make
    #time mpirun n0-15 ./debias.x $tmp/$basename.node%X.fs0 \
    time ${mpirun} ./debias.x $tmp/$basename.node%X.fs0 \
                                  $tmp/$basename.node%X.masked.fs0 -1

    echo --- Done with Step 3.1.2 - Sigmacut Debias ----------------------; date
}




# --- STEP 3.2.3 --- SVDRFI ----------
Step323() {
	cd $PROGDIR/SVDRFI
	echo --- Starting Step 3.2.3 - SVDRFI I1 -------------------------; date

	export KMP_STACKSIZE=2164608
	date
	echo ===== Starting svd_uvec ===========================================
	$mpirun ./svd_uvec.x $tmp/$basename.node%X.fs0 \
	    $OUTDIR/$basename.svdtvec.dat \
	    $OUTDIR/$basename.eval.dat
	echo ===== Done svd_uvec ===============================================
	date
	echo ===== Starting svd_vvec ===========================================
	$mpirun ./svd_vvec.x $tmp/$basename.node%X.fs0 \
	    $OUTDIR/$basename.svdtvec.dat \
	    $OUTDIR/$basename.svdrfi.node%X
	echo ===== Done svd_vvec ===============================================
	date
	#echo ===== Starting svd_model ==========================================
	#$mpirun ./svd_model.x $OUTDIR/$basename.svdtvec.dat \
	#    $OUTDIR/$basename.svdrfi.node%X \
	#    $OUTDIR/$basename.corrout.dat.node%X
	#echo ===== Done svd_model ==============================================
	date
	
	echo --- Done with Step 3.2.3 - SVDRFI I1 ------------------------; date
}


# --- Step 3.6 --- RFImaps ------------------
Step36() {
    echo --- Starting Step 3.6 - RFI maps ---------------------------; date

    svdrfi=${OUTDIR}/$basename.svdrfi.node%X
    #svdrfi=${PROGDIR}/Output/$basename.svdrfi.node%X

    startmode=1
    endmode=20  # about 1 min/mode

    cd $PROGDIR/RFImode
    make

    # this should be parallelized
    for mode in `seq $startmode $endmode`
      do
      omode=$(printf '%02d' $mode)
      echo "==================== $omode ===================="
      lag=$OUTDIR/$basename.lag.$omode.dat
      ./rfimode.x $svdrfi $OUTDIR/$basename $mode | tee $OUTDIR/$basename.$omode.out
      #./rfiposlag.x $lag $OUTDIR/$basename $mode | tee -a $OUTDIR/$basename.$omode.out
      ./rfiposlag_delay.x $lag $OUTDIR/$basename $mode | tee -a $OUTDIR/$basename.$omode.out
      
    #  cp $outdir/$basename.lagnoshift.$omode.dat lagnoshift.dat
    #  ./rfipos.x $lag $outdir/$basename $mode | tee -a $outdir/$basename.$omode.out
    #  mv chimap.pgm $outdir/$basename.chimap.$omode.pgm

    done

    ./split-tvec.sh $OUTDIR/$basename


    grep -a ' @' $OUTDIR/$basename.*.out | awk '{print $2,$3,$4,$1}' > $OUTDIR/$basename.coords.dat
    awk -f kml.awk $OUTDIR/$basename.coords.dat > $OUTDIR/$basename.kml
    
    echo --- Done Step 3.6 - RFI maps ---------------------------; date
}


# --- STEP 5 --- Backup --------------
Step5() {
    echo --- Starting Step 5 - Backup ---------------------------------; date
    
    # These exist only on the head node where the script runs
    mv $tmp/$basename.eval.dat $OUTDIR
    mv $tmp/$basename.svdtvec.dat $OUTDIR

    for ((i=0;i<=15;i++)) ; do
	x=`printf "%X" $i`
	move1="mv $tmp/$basename.svdrfi.node$x $OUTDIR"
	ssh node$(( $i + $node0 )) "$move1"
    done
    echo --- Done with Step 5 - Backup ---------------------------------; date; echo

}


# --- EXECUTION ---

date
echo running on `hostname`

Step1    # File Setup

#Step320  # Mask (not working yet)

Step323  # SVDRFI

Step36  # RFImaps

## Cleanup not needed when OUTDIR is on /mnt/code
#Step5  # Cleanup

date

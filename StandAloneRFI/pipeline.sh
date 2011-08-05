
#!/bin/sh +x
# Run SVD and make RFI maps 
# Adapted from similar program suite on NCRA HP cluster
# which was itself adapted from the CITA version
# Requires .fs0 files from fringestop on the dual nodes
# Update: We are switching node32 and node33 for gsbm2 and gsbm3. 
# 	Original in pipeline.old

#target=b2217
#rawtarget=${target}+47
target=b2045-16  #pulsar name, if SVDing EoR data, rfi if regular svd
rawtarget=${target}

month=jun
date=17
year=11
run=22
disk=EoRd


# Make sure OUTDIR is visible to all nodes or mount the OUTDIR on all nodes. Move the svd files to gsbm3:/mnt/EoR/gsbuser/EoR/RFI_Out/

#RFImode=RFImode
#SVDRFI=SVDRFI
RFImode=RFImode64
SVDRFI=SVDRFI64


# location of .fs0 files on each node (the result of decoding them data)
DATADIR=/mnt/${disk}/gsbuser/EoR
tmp=${DATADIR}/SVDRFI	#where files are put on nodes
basename=$month.$date.$year.$target.$run	# Naming convention
rawbasename=${rawtarget}_${month}${date}_${run}

PROGDIR=/mnt/code/gsbuser/EoR/StandAloneRFI
#PROGDIR=${0%/*}
OUTDIR=/mnt/EoR/gsbuser/EoR/RFI_Out/${basename}/
OUTMACHINE=gsbm3 # machine to copy SVD files to, where OUTDIR is located

node0=33
hostfile=$PROGDIR/dual_nodes

#mpirun="mpirun -np 16 -hostfile $hostfile"
#mpirun="time /opt/openmpi-1.2.2/bin/mpirun  -np 16 -bynode -hostfile $hostfile --mca btl_tcp_if_include eth2,eth3"
#mpirun="time /opt/openmpi-1.2.2/bin/mpirun  -np 16 -bynode -hostfile $hostfile --mca btl_tcp_if_include eth2"
mpirun="time /opt/openmpi-1.2.2/bin/mpirun  -np 16 -bynode -hostfile $hostfile "

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
     


	NT0=$(( $NT - ( $NT % 256 ) ))
	NTA=$(( $NT0 / 256 ))
	NT05=$(( $NT0 + 5 ))
	I="-i"
#        NT0=400
	#NT0=200 #changed as of 2011.07.31, attempt to fix july 12th scan 4

	sed $I "s/nt=[0-9]*/nt=$NT0/g" ${SVDRFI}/svd_model.f90 #| grep nt= 
	sed $I "s/nt=[0-9]*/nt=$NT0/g" ${SVDRFI}/svd_uvec.f90 #| grep nt= 
	sed $I "s/nt=[0-9]*/nt=$NT0/g" ${SVDRFI}/svd_vvec.f90 #| grep nt= 
	cd ${SVDRFI}; make; cd $PROGDIR


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
	cd $PROGDIR/${SVDRFI}
	echo --- Starting Step 3.2.3 - SVDRFI I1 -------------------------; date

	export KMP_STACKSIZE=2164608
 	date
 	echo ===== Starting svd_uvec ===========================================
 	$mpirun ./svd_uvec.x $tmp/$basename.node%X.fs0 \
 	    $tmp/$basename.svdtvec.dat \
 	    $tmp/$basename.eval.dat
 	echo ===== Done svd_uvec ===============================================
 	date
 	echo ===== Starting svd_vvec ===========================================
 	$mpirun ./svd_vvec.x $tmp/$basename.node%X.fs0 \
 	    $tmp/$basename.svdtvec.dat \
 	    $tmp/$basename.svdrfi.node%X   
 	echo ===== Done svd_vvec ===============================================
 	date
	#echo ===== Starting svd_model ==========================================
	#$mpirun ./svd_model.x $OUTDIR/$basename.svdtvec.dat \
	#    $OUTDIR/$basename.svdrfi.node%X \
	#    $OUTDIR/$basename.corrout.dat.node%X
	#echo ===== Done svd_model ==============================================
	date
	
	ssh $OUTMACHINE "scp 192.168.16.33:$tmp/$basename.svdtvec.dat $OUTDIR/"
	echo "copied $tmp/$basename.svdtvec.dat"
	ssh $OUTMACHINE "scp 192.168.16.33:$tmp/$basename.eval.dat $OUTDIR/"
	echo "copied $tmp/$basename.eval.dat"
	for node in `seq 33 48`
	do
	  ssh $OUTMACHINE "scp 192.168.16.${node}:$tmp/$basename.svdrfi.node'*' $OUTDIR/"
	  echo "copied $tmp/$basename.svdrfi.node'*'"

	done

	echo --- Done with Step 3.2.3 - SVDRFI I1 ------------------------; date
}


# --- Step 3.6 --- RFImaps ------------------
Step36() {
    echo --- Starting Step 3.6 - RFI maps ---------------------------; date

    echo "Assuming we are on $OUTMACHINE"

    svdrfi=${OUTDIR}/$basename.svdrfi.node%X
    #svdrfi=${PROGDIR}/Output/$basename.svdrfi.node%X

    startmode=1
    endmode=20  # about 1 min/mode

    cd $PROGDIR/${RFImode}
    make

    # this should be parallelized
    for mode in `seq $startmode $endmode`
      do
      omode=$(printf '%02d' $mode)
      echo "==================== $omode ===================="
      lag=$OUTDIR/$basename.lag.$omode.dat
      ./rfimode.x $svdrfi $OUTDIR/$basename $mode | tee $OUTDIR/$basename.$omode.out

      # OPTION 1
      ./rfiposlag.x $lag $OUTDIR/$basename $mode | tee -a $OUTDIR/$basename.$omode.out

      # OPTION 2
#      ./rfiposlag_delay.x $lag $OUTDIR/$basename $mode | tee -a $OUTDIR/$basename.$omode.out
      
      # OPTION 3
  #    cp $OUTDIR/$basename.lagnoshift.$omode.dat lagnoshift.dat
   #  ./rfipos.x | tee -a $OUTDIR/$basename.$omode.out
    # mv chimap.pgm $OUTDIR/$basename.chimap.$omode.pgm


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


#Step1    # File Setup

#Step320  # Mask (not working yet)

#Step323  # SVDRFI

Step36  # RFImaps

## Cleanup not needed when OUTDIR is on /mnt/code
#Step5  # Cleanup (old style, not needed anymore)

date

#echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
#echo "MOVE $OUTDIR TO GSBM3:/mnt/EoR/gsbuser/EoR/RFI_Out"
#echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"


# Not sure this actually works... also not necessary if OUTMACHINE=gsbm3 
# echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
# echo "MOVING $OUTDIR TO GSBM3:/mnt/EoR/gsbuser/EoR/RFI_Out"
# echo "SUPPLY THE PASSWORD AND WAIT FOR 'Done'"
# echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"

 if [[ scp -rp "$OUTDIR" "gsbm3:/mnt/EoR/gsbuser/EoR/RFI_Out" ]]; then
 	rm -rf "$OUTDIR"
 else
 	echo "You failed to supply the password"
 	exit 1
 fi

echo "We are Done"
exit

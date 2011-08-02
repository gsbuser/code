#!/bin/csh -x
#make
setenv DEBUG_PC 0
date

set op_mode = 0

set home_dir = $cwd
echo $home_dir

set acq_bit = 8     # sampling bit resolution
set clk =  33   # samping clock (MSPS)
set dump_bit = 4 # raw voltage data resolution written into the disk
set buffer_count = 2 # Duration of observation in secs
#set nint_visibility = 9
#set buffer_w_recal = 1
#set fstop = 0

/opt/openmpi-1.2.2/bin/mpirun --mca btl_tcp_if_include eth0,eth1 -n 32 -hostfile test_nodes_raw run_rawdump $op_mode $acq_bit $clk $dump_bit $buffer_count
date
echo " "
echo "Running gain equalization routine... new gain table in /home/gsbuser/gain/gaintab.dat of nodes 1 to 8"
echo " "
#for((i=1;i<9; i++)); do echo -n node${i}; ssh node${i} "cat ~/gain/gaintab.dat"; done
#for ((i=1;i<9; i++)); do echo node${i};ssh node${i} /home/gsbuser/gain/./gain_5sigma.for; done

#Allows user to input drive to save in, file name and log name. All other inputs needed are already filled in
#comment in line from run.csh, comment out fake-correlator

echo -n "Please enter the drive to save in [ie: EoRa]"
read drive_name
echo -n "Please enter the name of the file": 
read file_name
echo -n "Please enter what you would like to log it as:"
read log_name 

echo "/opt/openmpi-1.2.2/bin/mpirun --mca btl_tcp_if_include eth0,eth1 -n 48 -bynode -hostfile test_nodes  mpi_dual   /mnt/${drive_name}/gsbuser/EoR/${file_name}.node%d%s >& ${log_name} &"

#jobs

#./fake-correlator.sh >& $log_name &

#wait




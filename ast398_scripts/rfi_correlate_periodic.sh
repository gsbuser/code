#!/bin/bash

BASEFILENAME="rfi_jul07"
i=1 #iteration number
drive_name="EoRa" # harddrive to store stuff on
L=7200   # Total time to run rfi_correlate in seconds
I=300 # Length of each observing section in seconds
P=0   # Length of time to pause between observations in seconds

ST=$(date "+%s") # Start time
#IT=$(date "+%s")  # Start observing time
IT=$((i*I - I + ST))
echo "Starting mpirun, run $i at $(date)"

file_name="${BASEFILENAME}_${i}" # basefile name
log_name="LOG_${file_name}"

/opt/openmpi-1.2.2/bin/mpirun --mca btl_tcp_if_include eth0,eth1 -n 48 -bynode -hostfile test_nodes  mpi_dual   /mnt/${drive_name}/gsbuser/EoR/${file_name}.node%d%s >& ${log_name} &

echo "running with ${drive_name} ${file_name} ${log_name} at $(($(date "+%s") - $ST)) seconds in."

while [[ $(($(date "+%s") - $ST)) -lt $L ]]; do
  T=$(date "+%s")
  if [[ $((T - IT)) -lt $I ]]; then
    #echo "Doing it"
    sleep 0
  else
    echo "Killing mpirun, run $i at $(($(date "+%s") - $ST)) seconds in."
    
    kill $(ps -o pid -C mpirun | grep -v PID)
    
    echo "Pausing for $P seconds, at $(($(date "+%s") - $ST)) seconds in."
    sleep $P
    i=$((i + 1))  # Next Run
    
    if [[ $(($(date "+%s") - $ST)) -le $L ]]; then
      echo "Starting run $i at $(date)"
    
      file_name="${BASEFILENAME}_${i}" # basefile name
      log_name="LOG_${file_name}"
      /opt/openmpi-1.2.2/bin/mpirun --mca btl_tcp_if_include eth0,eth1 -n 48 -bynode -hostfile test_nodes  mpi_dual   /mnt/${drive_name}/gsbuser/EoR/${file_name}.node%d%s >& ${log_name} &

      echo "running with ${drive_name} ${file_name} ${log_name} at $(($(date "+%s") - $ST)) seconds in."
    
      #IT=$(date "+%s")
      IT=$((i*I - I + ST))
    fi
  fi
done

echo "Done at $(date)"

#echo "Killing last one at $(($(date "+%s") - $ST))"
#kill $(ps -o pid -C mpirun | grep -v PID)

exit


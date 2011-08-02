echo " "
echo "Running gain equalization routine... new gain table in /home/gsbuser/gain/gaintab.dat of nodes 1 to 16"
echo " "
for ((i=1;i<17; i++)); do echo node${i};ssh node${i} /home/gsbuser/gain/./gain_10sigma.for; done

#Hans Nguyen 2011/06/17
#Displays the available space of all five possible locations for fringestop
#data storage

#echo "gsbm2: /mnt/a $(ssh gsbm2 df -h | grep /mnt/a | awk '{print $4}') free"
echo "gsbm2: /mnt/a $(ssh gsbm2 df -h | grep /dev/sdc1 | awk '{print $4}') free"  

echo "gsbm2: /mnt/b $(ssh gsbm2 df -h | grep /mnt/b | awk '{print $4}') free"

echo "gsbm3: /mnt/a $(ssh gsbm3 df -h | grep /mnt/a | awk '{print $4}') free"
echo "gsbm3: /mnt/b $(ssh gsbm3 df -h | grep /mnt/b | awk '{print $4}') free"
echo "gsbm3: /mnt/EoR $(ssh gsbm3 df -h | grep /mnt/EoR | awk '{print $4}') free"

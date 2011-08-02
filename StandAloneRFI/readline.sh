hosts=$(cat dual_nodes)
hostarray=( )
for host in ${hosts[@]}
do
  hostarray=( ${hostarray[@]} $host )
done

for n in `seq 0 $(( ${#hostarray[@]} - 1 ))`
do
  X=$(printf "%X" $n)
  echo "host $n or $X: ${hostarray[$n]}"
done

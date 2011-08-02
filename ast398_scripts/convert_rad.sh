#Converts coordinates in either hours or degrees into radians for use in params.h
# Hans Nguyen
# 2011/06/16

# takes in 4 arguments

a=$1 #hours/degrees
b=$2 #min
c=$3 #sec
d=$4 #'d' or 'h' for hours or degrees

tmp=$(echo "(($c/60+$b)/60+$a)" | bc -l)

if [ "$d" == "d" ]
then
    rad=$(echo "$tmp*0.01745329251994329576923690768489" | bc -l)
elif [ "$d" == "h" ]
then
    rad=$(echo "$tmp*0.26179938779914943653855361527329" | bc -l)
else
    echo "fourth parameter must be either 'h' or 'd'"

fi

echo "$rad rad"

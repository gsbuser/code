#!/bin/bash
# Written by Joshua Albert
# 
# Function: 
# Overlaps greyscale images (.pgm, .pnm, .pam, .pbm, ect.) onto of one another. The net result is that the parts of the image that are common will remain in the output images. This helps find similarities between RFI_chimaps.
# 
#Output: 
#Two files. 'final.{filename}' and 'finalnothresh.{filename}'. The first one is in black and white and is useful when input files are fuzzy as it results in sharper results possibly at the cost of data integrity. The second file is without thresholding and will keep all input image data itegrity but the results may be indefinite or hardly an improvment over the original input files.

# Parameters: 
# OPAC: The OPAC variable is how much opacity the images have. The higher the OPAC the less of each previous image in the overlay procedure is retained. The lower the OPAC the more of each previous image is retained. For the 'final.{filename' OPAC=0.0 means the result will only be the first image (as all overlay'd images had no opacity). Likewise, OPAC=1.0 means the result will only be the last image (as all overlay'd images had complete opacity and only the last one will be seen). In 'finalnothresh.{filename}' OPAC=0.0 means the same but OPAC=1.0 dosen't necessarily mean complete opacity of overlay'd images. In general they will follow the same trend though.
# INVERT: The INVERT variable is simple to use. If white simbolizes data and black the background in the image files then set this to 'Y'. If black or darker colours signify data then set this 'N'. This is so we know what parts to mask out.
# THRESH: This is a number between 0.0 and 1.0 and allows us to control what we define as data. If it equals 0.5 for instance then in each image if the intensity is 50% the max or more then it will be called black and everything else will be called white. If you want to keep the most stucture in each image and are not sure which threshold level to use then perhaps you'd prefer the computer to choose for you. Then set THRESH='A'.

#Parameters (see above for details)
OPAC=0.6 # Between (0.0 , 1.0)
INVERT="Y" # 'Y' or 'N'
THRESH='A'

# Dont change below unless you want to

if [[ -z $1 ]]; then
        echo "Give file base name"
        exit 1
fi

echo "Doing overlap on $1"
#ls $1*

echo "Thresholding files:"
for f in $1*; do
	echo "  Thresholding: $f"
	if [[ "$THRESH" == "A" ]]; then
		pamthreshold $f > $f.th
	else
		pamthreshold -simple -threshold=$THRESH $f > $f.th
	fi
done
echo "Done thresholding"

echo "Overlapping files (with threshold):"
F1="T"
for f in $1*.th; do	
	if [[ "$F1" != "T" ]]; then
		echo "  Overlapping $f onto $f_prev with opacity: $OPAC"
		if [[ "$INVERT" == "Y" ]]; then
			pamcomp -invert -alpha=$f -opacity=$OPAC $f $f_prev $f.comp	
		else
			pamcomp -alpha=$f -opacity=$OPAC $f $f_prev $f.comp
		fi
		f_prev="$f.comp"
	else
		f_prev="$f"
		F1="F"		
	fi
done
echo "Done overlapping (with threshold)"

echo "Transofrming to output: final.$1"
pamtopnm $f_prev > "final.$1"

echo "  Cleaning up (threshold and compare files)"
rm -f $1*.th
rm -f $1*.th.comp

echo "Overlapping files (no threshold):"
F1="T"
for f in $1*; do	
	if [[ "$F1" != "T" ]]; then
		echo "  Overlapping $f onto $f_prev with opacity: $OPAC"
		if [[ "$INVERT" == "Y" ]]; then
			pamcomp -invert -alpha=$f -opacity=$OPAC $f $f_prev $f.comp	
		else
			pamcomp -alpha=$f -opacity=$OPAC $f $f_prev $f.comp
		fi
		f_prev="$f.comp"
	else
		f_prev="$f"
		F1="F"		
	fi
done
echo "Transforming to output: finalnothresh.$1"
pamtopnm $f_prev > "finalnothresh.$1"

echo "  Cleaning up (compare files)"
rm -f $1*.comp

echo "Done. Now look at 'final.$1' and 'finalnothresh.$1'"

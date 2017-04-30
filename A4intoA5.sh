#!/bin/bash
set -e
#set -x

if [ $# -lt 3 ]
  then
    echo "Usage: $0 input.pdf marg_x_mm marg_y_mm"
    exit
fi

if [ ! -f $1 ]
then
    echo "Error! $1 does not exist!"
    exit
fi

mkdir -p tmp crops out 

#original format
w_mm=210
h_mm=297
let "w=w_mm*17/6"
let "h=h_mm*17/6"

#small screen proportions
prop="15/20"

#margin as parametres (in mm)
mx=$2
my=$3
#into bigpoint unit
let "mx = mx*17/6"
let "my = my*17/6"
#used zone
let "ux=w-mx*2"
let "uy=ux*$prop"
#overlap for information
let "overlap=2*my+2*uy-h"
let "overlap=overlap*6/17"
echo "Overlap=" $overlap "mm"
#bbox1
let "a=mx"
let "b=h-my-uy"
let "c=mx+ux"
let "d=h-my"
pdfcrop --bbox "$a $b $c $d" $1 tmp/crop1.pdf
#bbox2
let "a=mx"
let "b=my"
let "c=mx+ux"
let "d=my+uy"
pdfcrop --bbox "$a $b $c $d" $1 tmp/crop2.pdf


pdftk tmp/crop1.pdf burst output crops/%04d_A.pdf
pdftk tmp/crop2.pdf burst output crops/%04d_B.pdf
cd crops
for f in *.pdf
do
pdftk $f cat 1-endwest output ../out/$f
done
cd ..
pdftk out/*.pdf cat output ${1%.pdf}_A5.pdf

echo "Finished! Output file: " ${1%.pdf}_A5.pdf
rm -r tmp crops out 


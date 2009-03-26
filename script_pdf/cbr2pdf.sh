#! /bin/bash

if ! [ $# -eq 1 ]; then
  echo "Usage : cbz2pdf file.cbz; Creates file.pdf; Needs : unrar, pdflatex"
else 

fichier="$1"
mkdir cbr2pdf_tmp
cd cbr2pdf_tmp

unrar e "../$fichier" 

echo "File : $fichier"

echo "\documentclass[a4paper, 12pt]{article} %article" > cbr2pdf.tex
echo "\usepackage[english, french]{babel}" >> cbr2pdf.tex
echo "\usepackage[utf8]{inputenc} " >> cbr2pdf.tex
echo "\usepackage{geometry}" >> cbr2pdf.tex
echo "\usepackage{epsfig}" >> cbr2pdf.tex
echo "\geometry{hmargin=0cm, vmargin=0cm }" >> cbr2pdf.tex
echo "\begin{document}" >> cbr2pdf.tex
echo " ">>cbr2pdf.tex

i=1

#change pic names to avoid latex problems => imgXX.jpg
for f in *.jpg
do
i=$(($i+1))
nom="img"
if [ $i -lt 10 ]; then
    nom=$nom"0"
fi
nom=$nom"$i".jp
mv "$f" "$nom"
done

for f in *.jp
do
mv "$f" "$f"g
done



for f in *.jpg
do
echo " ">>cbr2pdf.tex
echo "\begin{figure}" >> cbr2pdf.tex
echo "\begin{center}" >> cbr2pdf.tex
echo "	\includegraphics[height = 29.3cm]{$f}" >> cbr2pdf.tex
echo "\end{center}" >> cbr2pdf.tex
echo "\end{figure}" >> cbr2pdf.tex
echo "\clearpage" >> cbr2pdf.tex
done
echo " ">>cbr2pdf.tex
echo "\end{document}" >> cbr2pdf.tex

pdflatex cbr2pdf.tex > out
mv cbr2pdf.pdf ../"${fichier%.cbr}".pdf


cd ..

rm -r cbr2pdf_tmp

fi

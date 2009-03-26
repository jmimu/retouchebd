#! /bin/bash

if ! [ $# -eq 1 ]; then
  echo "Usage : cbz2pdf file.cbz; Creates file.pdf; Needs : unzip, pdflatex"
else 

fichier="$1"

unzip -j "$fichier" -d cbz2pdf_tmp

echo "File : $fichier"

cd cbz2pdf_tmp

echo "\documentclass[a4paper, 12pt]{article} %article" > cbz2pdf.tex
echo "\usepackage[english, french]{babel}" >> cbz2pdf.tex
echo "\usepackage[utf8]{inputenc} " >> cbz2pdf.tex
echo "\usepackage{geometry}" >> cbz2pdf.tex
echo "\usepackage{epsfig}" >> cbz2pdf.tex
echo "\geometry{hmargin=0cm, vmargin=0cm }" >> cbz2pdf.tex
echo "\begin{document}" >> cbz2pdf.tex
echo " ">>cbz2pdf.tex

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
echo " ">>cbz2pdf.tex
echo "\begin{figure}" >> cbz2pdf.tex
echo "\begin{center}" >> cbz2pdf.tex
echo "	\includegraphics[height = 29.3cm]{$f}" >> cbz2pdf.tex
echo "\end{center}" >> cbz2pdf.tex
echo "\end{figure}" >> cbz2pdf.tex
echo "\clearpage" >> cbz2pdf.tex
done
echo " ">>cbz2pdf.tex
echo "\end{document}" >> cbz2pdf.tex

pdflatex cbz2pdf.tex > out
mv cbz2pdf.pdf ../"${fichier%.cbz}".pdf


cd ..

rm -r cbz2pdf_tmp

fi

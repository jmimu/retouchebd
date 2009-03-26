#! /bin/bash

mkdir -p out


for f in *.cbz
do
if [ "$f" = "*.cbz" ]; then
    echo "No cbz files"
else
./cbz2pdf.sh "$f"
mv "${f%.cbz}".pdf out/
fi
done

for f in *.zip
do
if [ "$f" = "*.zip" ]; then
    echo "No zip files"
else
./cbz2pdf.sh "$f"
mv "${f%.zip}".pdf out/
fi
done

for f in *.cbr
do
if [ "$f" = "*.cbr" ]; then
    echo "No cbr files"
else
./cbr2pdf.sh "$f"
mv "${f%.cbr}".pdf out/
fi
done

for f in *.rar
do
if [ "$f" = "*.rar" ]; then
    echo "No rar files"
else
./cbr2pdf.sh "$f"
mv "${f%.rar}".pdf out/
fi
done



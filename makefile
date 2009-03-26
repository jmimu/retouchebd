OPTIONS = -Wall -g -O0
#OPTIONS = -O9
#SFML = -lsfml-graphics -lsfml-window -lsfml-system
#SDL = -lSDL
OPENCV = -lcxcore -lcv -lhighgui -lcvaux  
LIB = $(SDL) $(SFML) $(OPENCV)

all:retouche_BD

retouche_BD:main.o
	g++ -o $@ $^ $(OPTIONS) $(LIB)

main.o:main.cpp
	g++ -c $(OPTIONS) main.cpp

clear:
	rm -f *.o

remake:clear all


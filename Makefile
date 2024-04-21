.PHONY: dest clean

color-rotate: dest/color-rotate.o
	gcc -o color-rotate dest/color-rotate.o -lm

dest/color-rotate.o: color-rotate.c globals.h
	gcc -c color-rotate.c -o dest/color-rotate.o -lm

dest/fs.o: fs.c globals.h
	gcc -c fs.c -o dest/fs.o

dest/colorUtils.o globals.h: colorUtils.c
	gcc -c colorUtils.c -o dest/colorUtils.o


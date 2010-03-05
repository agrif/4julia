# Makefile

.PHONY : all clean

all : 4julia

clean :
	make -C core clean

4julia : core/*.c
	make -C core all


# core/Makefile

.PHONY : all clean

OBJECTS = core.o math.o raytracer.o image.o julia.o live.o main.o
TARGET = ../4julia

CFLAGS += -O3
LDFLAGS += -lm -lpng -lpthread -lSDL -lreadline

all : ${TARGET}

clean :
	rm -rf ../4julia ${OBJECTS}

${TARGET} : ${OBJECTS}
	gcc ${OBJECTS} ${LDFLAGS} -o ${TARGET}


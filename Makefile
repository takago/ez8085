MAKE = make
RM = rm -fr
CC = gcc
CFLAGS = -ggdb3 -W -Wall

.c.o	:
	${CC} -c $< -o $@ ${CFLAGS} 

OBJ = cpu8085.o debug.o iset8085.o symbol.o
EXEC =	ez8085

all:	${EXEC}

${EXEC}	: ${OBJ}
	 ${CC} -o $@ ${OBJ}

clean	:
	${RM} ${OBJ} ${EXEC} *~

cpu8085.o: cpu8085.c common.h iset8085.h cpu8085.h
debug.o: debug.c cpu8085.h common.h symbol.h iset8085.h
iset8085.o: iset8085.c common.h iset8085.h
symbol.o: symbol.c symbol.h

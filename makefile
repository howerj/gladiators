LDFLAGS  = -lglut -lGL -lm
CFLAGS   = -std=c99 -Wall -Wextra -g
RM      := rm
SOURCES := ${wildcard *.c}
OBJECTS := ${SOURCES:%.c=%.o}
DEPS    := ${SOURCES:%.c=%.d}
CFLAGS  += -MMD
TARGET  := arena

all: ${TARGET}

%.o: %.c
	@echo ${CC} $< -c -o $@
	@${CC} ${CFLAGS} $< -c -o $@

${TARGET}: ${OBJECTS}
	@echo ${CC} $< -o $@
	@${CC} ${CFLAGS} $^ ${LDFLAGS} -o $@

run: ${TARGET}
	./${TARGET}

gladiator.conf: ${TARGET}
	./${TARGET} -s

-include ${DEPS}

clean:
	${RM} -f *.o *.d *.out ${TARGET}

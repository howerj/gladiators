LDFLAGS  = -lglut -lGL -lm
CFLAGS   = -std=c99 -Wall -Wextra -g -O2
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

doc: gladiator.htm

gladiator.htm: readme.md
	markdown $^ | tee $@

-include ${DEPS}

clean:
	${RM} -f *.o *.d *.out ${TARGET} *.htm vgcore.* core

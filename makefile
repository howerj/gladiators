ifeq ($(OS),Windows_NT) # Windows MinGW
LDFLAGS  = -lfreeglut -lopengl32 -lm
else # Unixen
LDFLAGS  = -lglut -lGL -lm
endif
CFLAGS   = -std=c99 -Wall -Wextra -g -O2 
RM      := rm
SOURCES := ${wildcard *.c}
OBJECTS := ${SOURCES:%.c=%.o}
DEPS    := ${SOURCES:%.c=%.d}
CFLAGS  += -MMD
TARGET  := arena

.PHONY: all run clean

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

${TARGET}.1: readme.md
	pandoc -s -f markdown -t man $< -o $@

-include ${DEPS}

clean:
	${RM} -f *.o *.d *.out *.conf *.lsp ${TARGET} *.htm vgcore.* core

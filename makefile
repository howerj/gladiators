LDFLAGS  = -lglut -lGL -lm
CFLAGS   = -std=c99 -Wall -Wextra -g

# https://stackoverflow.com/questions/10520839/make-implicit-rules-and-header-files
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
	@${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

run: ${TARGET}
	./${TARGET}

-include ${DEPS}

clean:
	rm -f *.o *.d ${TARGET}

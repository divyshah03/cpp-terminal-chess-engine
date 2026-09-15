CXX ?= g++
CXXFLAGS = -std=c++20 -Wall -MMD -Werror=vla
LDFLAGS =
EXEC = chess
OBJECTS = board.o cell.o computerPlayer.o enumerated.o game.o \
          humanPlayer.o info.o main.o move.o \
          piece.o player.o position.o subject.o textDisplay.o timer.o

DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC} 

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}

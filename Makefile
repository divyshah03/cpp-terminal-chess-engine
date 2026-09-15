CXX ?= g++
CXXFLAGS = -std=c++20 -Wall -MMD -Werror=vla
LDFLAGS =
EXEC = chess
OBJECTS = board.o cell.o computerPlayer.o enumerated.o game.o \
          humanPlayer.o info.o main.o move.o \
          piece.o player.o position.o subject.o textDisplay.o timer.o

# Optional X11 board window: make GRAPHICS=1 (needs the X11 development libraries)
ifdef GRAPHICS
CXXFLAGS += -DENABLE_GRAPHICS -I/opt/X11/include
OBJECTS += graphicsDisplay.o window.o
LDFLAGS += -L/opt/X11/lib -lX11
endif

DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC} 

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}

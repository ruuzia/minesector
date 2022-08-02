CC=g++

OBJS = main.cpp texture.cpp game.cpp button.cpp

OBJS_O = main.o texture.o game.o button.o

OBJ_NAME = main

CXXFLAGS = -Wall -Werror -pedantic -std=c++2a
LDFLAGS =
LDLIBS = -lSDL2 -lSDL2_image -lSDL2_ttf

all: $(OBJ_NAME)

debug: CXXFLAGS += -DDebug -g
debug: $(OBJ_NAME)

main.o: main.cpp common.h texture.h button.h game.h
texture.o: texture.cpp texture.h common.h
game.o: game.cpp game.h
button.o: button.cpp button.h

%.o: SRC += %.cpp

$(OBJ_NAME): $(OBJS_O)

clean:
	rm -f $(OBJS_O) $(OBJ_NAME)

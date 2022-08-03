CC=g++


OBJ_NAME = main

CXXFLAGS = -Wall -std=c++2a
LDFLAGS =
LDLIBS = -lSDL2 -lSDL2_image -lSDL2_ttf

all: $(OBJ_NAME)

debug: CXXFLAGS += -DDebug -g
debug: $(OBJ_NAME)

OBJS_O = main.o texture.o game.o button.o anim.o

COMMON_H = common.h
TEXTURE_H = texture.h $(COMMON_H)
BUTTON_H = button.h $(TEXTURE_H)
ANIM_H = anim.h $(TEXTURE_H)
GAME_H = game.h $(BUTTON_H) $(ANIM_H)

main.o: main.cpp $(GAME_H)
texture.o: texture.cpp $(TEXTURE_H)
game.o: game.cpp $(GAME_H)
button.o: button.cpp $(BUTTON_H)
anim.o: anim.cpp $(ANIM_H)

$(OBJ_NAME): $(OBJS_O)

clean:
	rm -f $(OBJS_O) $(OBJ_NAME)

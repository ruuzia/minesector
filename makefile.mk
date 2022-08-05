OS ?= Unix
CXX ?= g++

EXE = main

CXXFLAGS = -Wall -std=c++1z

INCLUDE_DIRS = -D_REENTRANT -I/usr/include/SDL2
LDFLAGS =
LDLIBS = -lSDL2 -lSDL2_image -lSDL2_ttf 

INCLUDE_DIRS_WIN = -IC:\\mingw_dev\\include\\SDL2\\

LDLIBS_WIN = -LC:\\mingw_dev\\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
LDFLAGS_WIN =
CXX_WIN = g++.exe
CXXFLAGS_WIN = -Wall -std=c++1z
EXE_WIN = main.exe


all: $(EXE)

debug: CXXFLAGS += -DDebug -g
debug: $(OBJ_NAME)

OBJS_O = main.o texture.o game.o button.o anim.o

COMMON_H = src/common.h
TEXTURE_H = src/texture.h $(COMMON_H)
BUTTON_H = src/button.h $(TEXTURE_H)
ANIM_H = src/anim.h $(TEXTURE_H)
GAME_H = src/game.h $(BUTTON_H) $(ANIM_H)

%.o: src/%.cpp
ifeq ($(OS),Unix)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c -o $@ $<
else
	$(CXX_WIN) $(CXXFLAGS_WIN) $(INCLUDE_DIRS_WIN) -c -o $@ $<
endif

main.o: src/main.cpp $(GAME_H)
texture.o: src/texture.cpp $(TEXTURE_H)
game.o: src/game.cpp $(GAME_H)
button.o: src/button.cpp $(BUTTON_H)
anim.o: src/anim.cpp $(ANIM_H)



$(EXE): $(OBJS_O)
ifeq ($(OS),Unix)
	$(CXX) $(LDFLAGS) $(OBJS_O) $(LDLIBS) -o $(EXE)
else
	$(CXX_WIN) $(LDFLAGS_WIN) $(OBJS_O) $(LDLIBS_WIN) -o $(EXE_WIN)
endif

clean:
	rm -f $(OBJS_O) $(EXE) $(EXE_WIN)

.PHONY: all

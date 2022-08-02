#ifndef GAME_H
#define GAME_H

#include "button.h"

#include <vector>
#include <functional>
#include <random>


#define NUMBER_TILES_COUNT 8
#define MAX_COLS 24
#define MAX_ROWS 24

class Game;

class Tile : public Button {
public:
    void onClick();

    Texture *overlay;

    void render(SDL_Renderer *renderer);

    Tile(Texture *tex = nullptr);

    bool isMine() const { return mine; }
    bool isEmpty() const { return !mine; }

    bool isHidden() const { return hidden; }
    bool isRevealed() const { return !hidden; }

    bool isFlagged() const { return flagged; }
    bool isUnflagged() const { return !flagged; }

    void setMine(bool f) { mine = f; }
    void setFlagged(bool f) { flagged = f; }
    void setHidden(bool f) { hidden = f; }

    int number;

    bool isRed;

    void setGame(Game *parent) { game = parent; }

    void updateTexture(bool mouseOver = false);
    void foreach_touching_tile(std::function<void(Tile&)> callback, bool diagonals = true);
    int countTouchingMines();

    void reveal();

    int row;
    int col;
private:
    bool mine;
    bool hidden;
    bool flagged;

    Game *game;
};

typedef enum {
    TTEX_BLANK_SQUARE = 0,
    TTEX_HIDDEN,
    TTEX_HIGHLIGHT,
    TTEX_RED_SQUARE,
    COUNT_TTEX,
} TileTexture;

typedef enum {
    ICON_FLAG = 0,
    ICON_MINE,
    COUNT_ICONS,
} TileIcons;

typedef enum {
    GAME_READY = 0,
    GAME_STARTED = 1,
    GAME_WON = 2,
    GAME_LOST = 4,
    GAME_OVER = GAME_WON | GAME_LOST,
} GameState;

#define COUNT_TILE_NUMBERS 8

class Game {
public:
    Game(int rows = 12, int cols = 18);
    ~Game();

    int state;

    // Renderer and window are global

    bool loadMedia(SDL_Window *window);
    bool initialRender();

    void OnUpdate(int dt);
    void OnStart();

    void onMouseButtonDown(SDL_MouseButtonEvent &e);
    void onMouseButtonUp(SDL_MouseButtonEvent &e);

    void onMouseMove(SDL_Event *e, int x, int y);

    int rows, cols;

    std::vector<std::vector<Tile> > board;

    struct {
        Texture title;
        Texture tiles[COUNT_TTEX];
        Texture icons[COUNT_ICONS];
        Texture numbers[COUNT_TILE_NUMBERS];
    } textures;


private:

    bool started;

    bool hasWon();
    void generateStartingArea(Tile& tile);
    void generateMines();
    void flipTiles(Tile& root, int& count, bool diagonals = true);
    void onRevealTile(Tile& tile);

    SDL_Renderer *renderer;
    SDL_Window *window;


    // Font class?
    //
    TTF_Font* mainFont;

    Tile *currentHover;

    std::mt19937 rng;
};

#endif

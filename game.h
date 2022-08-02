#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "texture.h"
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

    Tile(Texture *tex = nullptr);

    bool isMine;
    bool isHidden;
    bool isFlagged;
    int number;

    void setGame(Game *parent) { game = parent; }

    void updateTexture(bool mouseOver = false);
    void foreach_touching_tile(std::function<void(Tile&)> callback);

    int row;
    int col;
private:
    Game *game;
};

typedef enum {
    TTEX_NUMBER_0 = 0,
    TTEX_NUMBER_8 = 8,
    TTEX_HIDDEN,
    TTEX_HIGHLIGHT,
    TTEX_CLICKING,
    TTEX_FLAG,
    TTEX_MINE,
    COUNT_TTEX,
} TileTexture;

class Game {
public:
    Game(int rows = 12, int cols = 18);
    ~Game();

    // Renderer and window are global

    bool loadMedia(SDL_Window *window);
    bool initialRender();

    void OnUpdate(int dt);
    void OnStart();

    void onMouseButtonDown(SDL_MouseButtonEvent &e);
    void onMouseButtonUp(SDL_MouseButtonEvent &e);

    void onMouseMove(SDL_Event *e, int x, int y);

    int rows, cols;

    std::vector<std::vector<Tile> > tiles;

    struct {
        Texture title;
        Texture tile[COUNT_TTEX];
    } textures;


private:

    bool started;

    void generateStartingArea(Tile& tile);
    void generateMines();
    int countTouchingMines(Tile& tile);

    SDL_Renderer *renderer;
    SDL_Window *window;

    void foreach_touching_tile(Tile& tile, std::function<void(Tile&)> callback);

    // Font class?
    //
    Texture *hoverTexture;

    TTF_Font* mainFont;

    Tile *currentHover;

    std::mt19937 rng;

    const int TITLE_FONT_SIZE = 40;
    const int TITLE_SPACE_ABOVE = 15;
    const int TITLE_SPACE_BELOW = 45;
    const int TILE_SIZE = 32;

    const float MINE_RATIO = 1.0 / 4.0;

};

#endif

#ifndef GAME_H
#define GAME_H

#include "button.h"
#include "anim.h"

#include <vector>
#include <functional>
#include <random>
#include <memory>
#include <deque>


#define NUMBER_TILES_COUNT 8
#define MAX_COLS 24
#define MAX_ROWS 24

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


class Game;

class Tile : public Button {
public:
    static Texture backgrounds[COUNT_TTEX];
    static Texture overlays[COUNT_ICONS];
    static Texture numbers[NUMBER_TILES_COUNT];
    static void loadMedia();

    Texture *overlay;

    void render();

    Tile(Texture *tex = nullptr);

    bool isMine() const { return mine; }
    bool isSafe() const { return !mine; }

    bool isHidden() const { return hidden; }
    bool isRevealed() const { return !hidden; }

    bool isFlagged() const { return flagged; }
    bool isUnflagged() const { return !flagged; }

    void setMine(bool f) { mine = f; }
    void setFlagged(bool f) { flagged = f; }
    void setHidden(bool f) { hidden = f; }

    void flag();
    void unflag();
    void flip(bool flipNeighbors = true, Uint32 delay = 0);
    void reset();
    void red();
    void mouseEnter();
    void mouseLeave();

    void OnUpdate(double dt) {
        render();
        animState.update(dt);
    }

    int number;

    bool isRed;

    void setGame(Game *parent) { game = parent; }

    void foreach_touching_tile(std::function<void(Tile&)> callback, bool diagonals = true) const;
    int countTouchingMines() const;

    int row;
    int col;
    AnimState animState;

private:
    bool mine;
    bool hidden;
    bool flagged;

    void playFlagAnim();
    void playUncoverAnim(Uint32 delay);

    Game *game;

    static const int TILE_SIZE = 32;
};

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

    void loadMedia(SDL_Window *window);
    bool initialRender();

    void OnUpdate(double dt);
    void OnStart();

    void onMouseButtonDown(SDL_MouseButtonEvent const &e);
    void onMouseButtonUp(SDL_MouseButtonEvent const &e);

    void onMouseMove(SDL_MouseMotionEvent const& e);

    int rows, cols;

    std::vector<std::vector<Tile> > board;

    struct {
        Texture title;
        Texture playAgain;
    } textures;

    std::mt19937 rng;

private:
    TextButton playAgainBtn;
    bool started;

    void restartGame();
    void onLost(Tile& mine);
    bool hasWon();
    void generateStartingArea(Tile& tile);
    void generateMines();
    void flipTiles(Tile& root, int& count, std::vector<Tile*>& toreveal, bool diagonals = true);
    void onRevealTile(Tile& tile);


    SDL_Window *window;


    // Font class?
    //
    TTF_Font* mainFont;

    Tile *currentHover;

};

#endif

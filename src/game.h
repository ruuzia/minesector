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

    explicit Tile(Texture *tex = nullptr);

    [[nodiscard]] bool isMine() const { return mine; }
    [[nodiscard]] bool isSafe() const { return !mine; }

    [[nodiscard]] bool isHidden() const { return hidden; }
    [[nodiscard]] bool isRevealed() const { return !hidden; }

    [[nodiscard]] bool isFlagged() const { return flagged; }
    [[nodiscard]] bool isUnflagged() const { return !flagged; }

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

    bool isRed;

    void setGame(Game *parent) { game = parent; }

    void foreach_touching_tile(std::function<void(Tile&)> callback, bool diagonals = true) const;
    int countTouchingMines() const;

    void forceUpdateTexture();
    Uint8 save();
    void load(Uint8 data);

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

enum GameState {
    READY = 0,
    STARTED = 1,
    WON = 2,
    LOST = 4,
    OVER = GameState::WON | GameState::LOST,
};

#define COUNT_TILE_NUMBERS 8

class Game {
public:
    Game(int rows, int cols);
    Game();
    ~Game();

    int state;

    // Renderer and window are global

    void loadMedia(SDL_Window *window);
    bool initialRender();

    void OnUpdate(double dt);
    void OnStart();
    void save();
    void load();

    void onMouseButtonDown(SDL_MouseButtonEvent const &e);
    void onMouseButtonUp(SDL_MouseButtonEvent const &e);

    void onMouseMove(SDL_MouseMotionEvent const& e);

    int rows, cols;

    std::vector<std::vector<Tile> > board;

    std::mt19937 rng;

    AnimState animState;
    void updateFlagCount();

    int mineCount;

private:
    int screen_width, screen_height;
    Text title;
    Text flagCounter;
    TextButton restartBtn;
    TextButton playAgainBtn;
    std::vector<TextButton> difficultyBtns;

    TextButton& activeRestartButton();

    void ready();
    void resizeBoard();
    void restartGame();
    void positionItems();
    void onLost(Tile& mine);
    bool hasWon();
    void generateStartingArea(Tile& tile);
    void generateMines();
    void flipTiles(Tile& root, int count, std::vector<Tile*>& toreveal);

    void onRevealTile(Tile& tile);

    SDL_Window *window;

    std::vector<Uint8> tileDatas;

    // Font class?
    //
    TTF_Font* mainFont;

    Tile *currentHover;

};

#endif

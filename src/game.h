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

enum TileBG {
    BLANK_SQUARE = 0,
    HIDDEN,
    HIGHLIGHT,
    RED_SQUARE,
    TILE_BG_COUNT,
};

enum TileOverlay {
    FLAG = 0,
    MINE,
    TILE_OVERLAY_COUNT,
};


class Game;

class Tile : public Button {
public:
    static Texture backgrounds[TILE_BG_COUNT];
    static Texture overlays[TILE_OVERLAY_COUNT];
    static Texture numbers[1 + NUMBER_TILES_COUNT];
    static void loadMedia(Font const& font);

    int getWidth() const override { return TILE_SIZE; }
    int getHeight() const override { return TILE_SIZE; }

    Texture *overlay;

    void render() override;

    explicit Tile(Texture *tex = nullptr);

    [[nodiscard]] bool isMine() const { return mine; }
    [[nodiscard]] bool isSafe() const { return !mine; }

    [[nodiscard]] bool isHidden() const { return hidden; }
    [[nodiscard]] bool isRevealed() const { return !hidden; }

    [[nodiscard]] bool isFlagged() const { return flagged; }
    [[nodiscard]] bool isUnflagged() const { return !flagged; }
    [[nodiscard]] bool exists() const { return !removed; }

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
    void dissapear();

    void OnUpdate(double dt) {
        render();
        animState.update(dt);
    }

    bool isRed;
    bool removed;

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
    Game(SDL_Window *window, int rows, int cols);
    Game(SDL_Window *window);
    ~Game();

    int state;

    // Renderer and window are global

    void loadMedia();
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
    void positionItems();

    int mineCount;

    char* saveDirectory;

    Font mainFont;
private:
    SDL_Window *window;

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
    void onLost(Tile& mine);
    void onWon();
    bool hasWon();
    void generateStartingArea(Tile& tile);
    void generateMines();
    void flipTiles(Tile& root, int count, std::vector<Tile*>& toreveal);

    void onRevealTile(Tile& tile);


    std::vector<Uint8> tileDatas;


    Tile *currentHover;

};

#endif

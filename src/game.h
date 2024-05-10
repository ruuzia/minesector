#ifndef GAME_H
#define GAME_H

#include "button.h"
#include "anim.h"
#include "tile.h"

#include <ctime>
#include <vector>
#include <array>
#include <functional>
#include <random>
#include <memory>
#include <deque>

enum GameState {
    READY = 0,
    STARTED = 1,
    WON = 2,
    LOST = 4,
    OVER = WON | LOST,
};

namespace SoundEffects {
    enum {
        FLAG    = 0,
        WHOOSH  = 1,
        BLIP    = 2,
        EXPLODE = 3,
        COUNT,
    };
}

namespace Icons {
    enum {
        SOUND = 0,
        MUTED = 1,
        COUNT,
    };
}

// Separate class for Field so we can change
// implementation easily
class Field {
public:
    Field(int num_rows, int num_cols, Game *game) : game(game) {
        resize(num_rows, num_cols);
    }
    int rows() const {
        return r;
    }
    int cols() const {
        return c;
    }
    Tile& get(int row, int col) {
        SDL_assert(row < rows() && col < cols());
        return tiles[row * cols() + col];
    }
    void resize(int num_rows, int num_cols) {
        r = num_rows;
        c = num_cols;
        tiles.resize(num_rows * num_cols);
        for (int row = 0; row < rows(); ++row) {
            for (int col = 0; col < cols(); ++col) {
                Tile& tile = get(row, col);
                tile.reset();
                tile.row = row;
                tile.col = col;
                tile.setGame(game);
            }
        }
    }

private:
    int r;
    int c;
    std::vector<Tile> tiles;
    Game *game;
};


class Game {
public:
    Game(SDL_Window *window);
    ~Game();

    // Renderer and window are global

    void loadMedia();
    bool initialRender();

    void OnUpdate(double dt);
    void OnStart();
    void save();
    void load();

    void onClick(int x, int y);
    void onAltClick(int x, int y);

    void onMouseMove(SDL_MouseMotionEvent const& e);

    int mouseX, mouseY;

    // std::array? why should I care?
    Field board;

    uint32_t seed;
    std::mt19937 rng;

    AnimState animState;
    void updateFlagCount();
    void positionItems();

    int mineCount;

    char* saveDirectory;

    static Mix_Chunk* sounds[SoundEffects::COUNT];

    Font mainFont;
    int state;

    Texture tileBackgrounds[TileBG::COUNT];
    Texture tileOverlays[TileOverlay::COUNT];
    Texture tileNumbers[NUMBER_TILES_COUNT];
private:
    SDL_Window *window;

    Text flagCounter;
    TextButton restartBtn;
    TextButton playAgainBtn;
    std::vector<TextButton> difficultyBtns;
    Button speakerBtn;

    std::vector<Button*> buttons;

    Texture icons[Icons::COUNT];

    TextButton& activeRestartButton();

    void ready();
    void restartGame();
    void onLost(Tile& mine);
    void onWon();
    bool hasWon();
    void generateStartingArea(Tile& tile);
    void generateMines();
    void flipTiles(Tile& root, int count, std::vector<Tile*>& toreveal);

    void onRevealTile(Tile& tile);

    std::vector<Uint8> tileDatas;
};

namespace Save {
    extern const char HEADER[];
    extern const char *FILE;
}

#endif

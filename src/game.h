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


#define MAX_FIELD_SIZE 50

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

    int rows, cols;

    int mouseX, mouseY;

    // std::array? why should I care?
    Tile board[MAX_FIELD_SIZE][MAX_FIELD_SIZE];

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
};

namespace Save {
    extern const char HEADER[];
    extern const char *FILE;
}

#endif

#ifndef GAME_H
#define GAME_H

#include "button.h"
#include "anim.h"
#include "tile.h"

#include <vector>
#include <functional>
#include <random>
#include <memory>
#include <deque>

enum GameState {
    READY = 0,
    STARTED = 1,
    WON = 2,
    LOST = 4,
    OVER = GameState::WON | GameState::LOST,
};

namespace SoundEffects {
    enum {
        FLAG = 0,
        WHOOSH,
        BLIP,
        EXPLODE,
        SHOVEL,
        COUNT,
    };
}


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

    static Mix_Chunk* sounds[SoundEffects::COUNT];

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

#ifndef TILE_H
#define TILE_H
#include <SDL_mixer.h>

#include "button.h"
#include "anim.h"

#define NUMBER_TILES_COUNT 8

// using enum in namespaces for benefits of enum class
// while letting me use them as indices

namespace TileBG {
    enum {
        BLANK_SQUARE = 0,
        HIDDEN,
        HIGHLIGHT,
        RED_SQUARE,
        COUNT,
    };
}

namespace TileOverlay {
    enum {
        FLAG = 0,
        MINE,
        COUNT,
    };
}

namespace TileSaveData {
    enum {
        HIDDEN = 1,
        MINE = 2,
        FLAGGED = 4,
        RED = 8,
        REMOVED = 16,

        DEFAULT = HIDDEN,
    };
}

class Game;

class Tile : public Button {
public:
    static Texture backgrounds[TileBG::COUNT];
    static Texture overlays[TileOverlay::COUNT];
    static Texture numbers[1 + NUMBER_TILES_COUNT];
    static void loadMedia(Font const& font);
    static void free();

    int getWidth() const override { return TILE_SIZE; }
    int getHeight() const override { return TILE_SIZE; }

    Texture *overlay;

    void render() override;

    Tile(Texture *tex = nullptr);

    Tile(Tile const& tile);

    ~Tile() = default;
    void operator=(Tile other);


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

    static constexpr int TILE_SIZE = 32;
};


#endif

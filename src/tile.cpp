#include "tile.h"
#include "game.h"

namespace Flag {
    namespace Rotation {
        // Base of flag poll
        constexpr double POINT_X = 18 / 64.0;
        constexpr double POINT_Y = 57 / 64.0;

        constexpr double START_DEGREES = 120;
        constexpr double DELTA_DEGREES = 270;
    }
}

namespace WinTile {
    constexpr double DELTA_ALPHA = -0.5;
}

namespace Uncover {
    constexpr double TIME = 1.0;
}

constexpr float NUMBER_SCALE = 0.8;

// C++17 standard compatible for now
// No array designators :(

std::string TILE_FILES[TileBG::COUNT] = {
    "assets/images/square_blank.png",
    "assets/images/tile.png",
    "assets/images/hovered_tile.png",
    "assets/images/square_red.png",
};

std::string ICON_FILES[TileOverlay::COUNT] = {
    "assets/images/flag.png",
    "assets/images/mine.png",
};

const Color NUMBER_COLORS[] = {
    0, // Number 0 has no text!
    0x1300d8,
    0x02850e,
    0xcb001e,
    0x130e46,
    0x003e14,
    0x460202,
    0x986207,
    0x7100c7,
};

enum TileAnim {
    FLAG_ANIM = 1,
    UNCOVER,
    REVEALMINE,
};

class FlagAnim : public Anim {
public:
    FlagAnim(const Texture *flagTex, SDL_Point pos, bool& isFlagged);

    bool OnUpdate(double dt) override;
    void OnStart() override;

private:
    const Texture* flag;
    SDL_Point pos;
    bool& isFlagged;
    double angle;
    SDL_Point rotPoint;
};

FlagAnim::FlagAnim(const Texture *flagTex, SDL_Point pos, bool&isFlagged)
    : flag(flagTex), pos(pos), isFlagged(isFlagged)
{
    using namespace Flag;
    rotPoint.x = (int)(Rotation::POINT_X * flag->getWidth());
    rotPoint.y = (int)(Rotation::POINT_Y * flag->getHeight());
}

void FlagAnim::OnStart() {
    using namespace Flag;
    angle = isFlagged ? Rotation::START_DEGREES : 0.0;
}


bool FlagAnim::OnUpdate(double dt) {
    using namespace Flag;

    if (angle < 0.0 || angle > Rotation::START_DEGREES) {
        return false;
    }
    if (isFlagged) angle -= Rotation::DELTA_DEGREES * dt;
    else angle += Rotation::DELTA_DEGREES * dt;
    
    flag->render(pos.x, pos.y, nullptr, angle, &rotPoint);
    return true;
}

class UncoverAnim : public Anim {
public:
    UncoverAnim(const Texture *hidden, SDL_Point pos, std::mt19937& rng);
    // Call virtual destructor, nothing to free
    ~UncoverAnim() {}

    void OnStart() override;
    bool OnUpdate(double dt) override;

private:
    const Texture *hidden;
    SDL_Point pos;
    std::mt19937 &rng;

    double widthPercent;
    double heightPercent;
    double deltaWidth;
    double deltaHeight;
    bool inverseX;
    bool inverseY;
};



UncoverAnim::UncoverAnim(const Texture *hidden, SDL_Point pos, std::mt19937& rng)
    : hidden(hidden)
    , pos(pos)
    , rng(rng)
    , widthPercent(1.0)
    , heightPercent(1.0)
    , deltaWidth(0.0)
    , deltaHeight(0.0)
    , inverseX(false)
    , inverseY(false)
{
}

void UncoverAnim::OnStart() {
    double v = std::uniform_real_distribution<>(0.0, 1.0)(rng);
    if (v < 0.5) {
        deltaWidth = Uncover::TIME;
        inverseX = v < 0.25;
    }
    else {
        deltaHeight = Uncover::TIME;
        inverseY = v < 0.75;
    }
}

bool UncoverAnim::OnUpdate(double dt) {
    widthPercent -= deltaWidth * dt;
    heightPercent -= deltaHeight * dt;

    if (widthPercent < 0 || heightPercent < 0) return false;

    SDL_Rect rect;
    rect.x = inverseX ? int(hidden->getWidth() * (1 - widthPercent)) : 0;
    rect.y = inverseY ? int(hidden->getHeight() * (1 - heightPercent)) : 0;
    rect.w = int(hidden->getWidth() * widthPercent);
    rect.h = int(hidden->getHeight() * heightPercent);

    hidden->renderPart(pos.x, pos.y, &rect, true);

    return true;
}

class WinTileAnim : public Anim {
public:
    WinTileAnim(SDL_Point pos, int size);
    ~WinTileAnim() override = default;

    void OnStart() override;
    bool OnUpdate(double dt) override;

private:
    SDL_Point pos;
    int size;
    Color color {0x008000};
    SDL_Rect fillrect { pos.x, pos.y, size, size };
};

WinTileAnim::WinTileAnim(SDL_Point pos, int size) : pos(pos), size(size)
{}

void WinTileAnim::OnStart() {
}

bool WinTileAnim::OnUpdate(double dt) {
    color.draw();
    SDL_RenderFillRect(renderer, &fillrect);

    color.a += dt * WinTile::DELTA_ALPHA;
    return color.a > 0.0;
}

Tile::Tile(Texture *tex) : Button(tex) {
    setHidden(false);
    setFlagged(false);
    setMine(false);
    row = 0;
    col = 0;
    isRed = false;
}

// WARNING: copy constructor and operator= don't actually copy fields rn
Tile::Tile(Tile const & t)
    : Tile()
{
    (void)t;
}
void Tile::operator=(Tile other) {
    (void)other;
}

Uint8 Tile::save() {
    using namespace TileSaveData;
    Uint8 data = 0;
    if (isHidden())   data |= HIDDEN;
    if (isMine())     data |= MINE;
    if (isFlagged())  data |= FLAGGED;
    if (isRed)        data |= RED;
    if (removed)      data |= REMOVED;
    return data;
}

void Tile::load(Uint8 data) {
    using namespace TileSaveData;
    setMine     (data & MINE);
    setHidden   (data & HIDDEN);
    setFlagged  (data & FLAGGED);
    isRed =      data & RED;
    removed =    data & REMOVED;
}

// No animations
void Tile::forceUpdateTexture() {
    if (removed) {
        background = nullptr;
        overlay = nullptr;
    }
    else if (isHidden()) {
        background = &backgrounds[TileBG::HIDDEN];
        if (isFlagged()) {
            overlay = &overlays[TileOverlay::FLAG];
        }
    }
    else {
        background = &backgrounds[isRed ? TileBG::RED_SQUARE : TileBG::BLANK_SQUARE];
        if (isMine()) {
            overlay = &overlays[TileOverlay::MINE];
        }
        else {
            overlay = &numbers[countTouchingMines()];
        }
    }
    animState.kill();
}

int Tile::countTouchingMines() const {
    int nearbyMines = 0;
    foreach_touching_tile([&](Tile& tile) -> void {
        if (tile.isMine()) {
            nearbyMines += 1;
        }
    });

    return nearbyMines;
}


void Tile::playFlagAnim() {
    if (animState.isAnimActive(TileAnim::FLAG_ANIM)) return;

    overlay = nullptr;
    background = &backgrounds[TileBG::HIDDEN];
    auto flagAnim = new FlagAnim(&overlays[TileOverlay::FLAG], {x, y}, flagged);
    flagAnim->onfinish = [this] () {
        if (flagged) overlay = &overlays[TileOverlay::FLAG];
    };
    animState.play(TileAnim::FLAG_ANIM, flagAnim);
}

void Tile::flag() {
    setFlagged(true);
    playFlagAnim();
    game->updateFlagCount();
}

void Tile::unflag() {
    setFlagged(false);
    playFlagAnim();
    game->updateFlagCount();
}

void Tile::mouseEnter() {
    if (isHidden() && isUnflagged()) {
        background = &backgrounds[TileBG::HIGHLIGHT];
    }
    else if (animState.isAnimActive(TileAnim::UNCOVER) && !animState.started) {
        // remove delay on uncover animation when user hovers over
        animState.startTime = SDL_GetTicks();
    }
}

void Tile::mouseLeave() {
    if (background == &backgrounds[TileBG::HIGHLIGHT]) {
        background = &backgrounds[TileBG::HIDDEN];
    }
}

void Tile::reset() {
    animState.kill();
    setMine(false);
    setFlagged(false);
    setHidden(true);
    background = &backgrounds[TileBG::HIDDEN];
    overlay = nullptr;
    isRed = false;
    removed = false;
}

void Tile::red() {
    isRed = true;
    background = &backgrounds[TileBG::RED_SQUARE];
}

void Tile::dissapear() {
    animState.play(-1, new WinTileAnim({x, y}, SIZE));
    background = nullptr;
    overlay = nullptr;
    removed = true;
}

int Tile::SIZE = 32;

void Tile::flip(bool recurse, Uint32 delay) {
    setHidden(false);
    if (isMine()) {
        overlay = nullptr;
        auto anim = new MineRevealAnim({x,y}, SIZE);
        anim->onstart = [this](){
            overlay = &overlays[TileOverlay::MINE];
            background = &backgrounds[TileBG::BLANK_SQUARE];
        };
        animState.play(TileAnim::REVEALMINE, anim, delay);
    }
    else {
        int neighboringMines = countTouchingMines();

        playUncoverAnim(delay);

        if (recurse && neighboringMines == 0) {
            // Recursively reveal surrounding tiles
            foreach_touching_tile([&recurse, &delay](Tile& tile) {
                if (tile.isHidden()) {
                    tile.flip(recurse, delay += 100);
                }
            });
        }
    }
}

void Tile::playUncoverAnim(Uint32 delay) {
    auto uncoverAnim = new UncoverAnim(&backgrounds[TileBG::HIDDEN], {x, y}, game->rng);
    // Use onstart to set textures to account for `delay` parameter
    uncoverAnim->onstart = [this]() {
        //Mix_PlayChannel(-1, game->sounds[SoundEffects::SHOVEL], 0);
        overlay = &numbers[countTouchingMines()];
        background = &backgrounds[TileBG::BLANK_SQUARE];
    };

    animState.play(TileAnim::UNCOVER, uncoverAnim, delay);

}

void Tile::foreach_touching_tile(std::function<void(Tile&)> callback, bool diagonals) const {
    // Rather verbose of me

    const int left = col - 1;
    const int right = col + 1;
    const int below = row + 1;
    const int above = row - 1;

    const bool spaceLeft = left >= 0;
    const bool spaceRight = right < game->cols;
    const bool spaceAbove = above >= 0;
    const bool spaceBelow = below < game->rows;

    auto& board = game->board;

    if (spaceLeft) callback(board[row][left]);
    if (spaceRight) callback(board[row][right]);
    if (spaceAbove) callback(board[above][col]);
    if (spaceBelow) callback(board[below][col]);

    if (diagonals) {
        if (spaceLeft && spaceAbove) callback(board[above][left]);
        if (spaceRight && spaceAbove) callback(board[above][right]);
        if (spaceLeft && spaceBelow) callback(board[below][left]);
        if (spaceRight && spaceBelow) callback(board[below][right]);
    }
}

void Tile::render() {
    if (background)
        background->render(x, y);

    if (overlay != nullptr) {
        overlay->render(x + (SIZE - overlay->getWidth()) / 2, y + (SIZE - overlay->getHeight()) / 2);
    }
}

Texture Tile::backgrounds[TileBG::COUNT];
Texture Tile::overlays[TileOverlay::COUNT];
Texture Tile::numbers[1 + NUMBER_TILES_COUNT];

void Tile::loadMedia(Font const& font) {
    for (int i = 0; i < TileBG::COUNT; ++i) {
        Tile::backgrounds[i].loadFile(TILE_FILES[i]);
    }

    for (int i = 0; i < TileOverlay::COUNT; ++i) {
        overlays[i].loadFile(ICON_FILES[i]);
    }
    overlays[TileOverlay::MINE].setMultColor(0.0, 0.0, 0.0);

    for (int i = 1; i <= COUNT_TILE_NUMBERS; ++i) {
        const char num[] = {char(i + '0'), '\0'};

        const Color color = NUMBER_COLORS[i];
        numbers[i].loadText(font.raw(), num, color.as_sdl());
    }

    reposition();
}

void Tile::reposition() {
    for (int i = 0; i < TileBG::COUNT; ++i) {
        Tile::backgrounds[i].setSize(SIZE, SIZE);
    }

    for (int i = 0; i < TileOverlay::COUNT; ++i) {
        overlays[i].setSize(SIZE, SIZE);
    }
    for (int i = 1; i <= COUNT_TILE_NUMBERS; ++i) {
        numbers[i].setScale(NUMBER_SCALE * (SIZE / 32));
    }
}

void Tile::free() {
}


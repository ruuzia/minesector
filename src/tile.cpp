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

    auto flagAnim = new FlagAnim(&game->tileOverlays[TileOverlay::FLAG], {x, y}, flagged);
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
    if (animState.isAnimActive(TileAnim::UNCOVER) && !animState.started) {
        // remove delay on uncover animation when user hovers over
        animState.startTime = SDL_GetTicks();
    }
}

void Tile::reset() {
    animState.kill();
    setMine(false);
    setFlagged(false);
    setHidden(true);
    isRed = false;
    removed = false;
}

void Tile::red() {
    isRed = true;
}

void Tile::dissapear() {
    animState.play(-1, new WinTileAnim({x, y}, SIZE));
    removed = true;
}

constexpr int TILE_BASE_SIZE = 32;
int Tile::SIZE = TILE_BASE_SIZE;

void Tile::flip(bool recurse, Uint32 delay) {
    setHidden(false);
    if (isMine()) {
        auto anim = new MineRevealAnim({x,y}, SIZE);
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
    auto uncoverAnim = new UncoverAnim(&game->tileBackgrounds[TileBG::HIDDEN], {x, y}, game->rng);

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

Texture *Tile::getBackground(bool isSelected) {
    using namespace TileBG;
    if (removed) return nullptr;
    if (isSelected && isClickable()) return &game->tileBackgrounds[HIGHLIGHT];
    if (isRed) return &game->tileBackgrounds[RED_SQUARE];
    if (isHidden() || animState.isAnimPending()) return &game->tileBackgrounds[HIDDEN];
    return &game->tileBackgrounds[BLANK_SQUARE];
}

bool Tile::isClickable() {
    return isHidden() && isUnflagged() && !(game->state & GameState::OVER);
}

Texture *Tile::getOverlay(void) {
    using namespace TileOverlay;
    if (removed) return nullptr;
    if (isHidden() && isFlagged() &&
        !animState.isAnimActive(TileAnim::FLAG_ANIM)) return &game->tileOverlays[FLAG];
    size_t neighbours = countTouchingMines();
    if (isRevealed() && isSafe() && !animState.isAnimPending(TileAnim::UNCOVER)) return neighbours == 0 ? nullptr : &game->tileNumbers[neighbours - 1];
    if (isRevealed() && isMine() && !animState.isAnimPending(TileAnim::REVEALMINE)) return &game->tileOverlays[MINE];
    return nullptr;
}

void Tile::render(bool isSelected) {
    Texture *bg = getBackground(isSelected);
    Texture *fg = getOverlay();

    if (bg) bg->render(x, y);
    if (fg) {
        fg->render(x + (SIZE - fg->getWidth()) / 2, y + (SIZE - fg->getHeight()) / 2);
    }
}

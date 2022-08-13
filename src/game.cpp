#include "common.h"
#include "game.h"
#include <string>
#include "color.h"
#include <algorithm>

constexpr int TITLE_SPACE_ABOVE = 15;
constexpr int TILE_SIZE = 32;
constexpr float PERCENT_MINES = 0.15;

constexpr int STARTING_SAFE_COUNT = 15;

constexpr struct {int cols; int rows; } SIZES[] = { {10, 8}, {15, 12}, {20, 15} };
const std::string DIFFICULTY_STRINGS[] = {"Easy", "Medium", "Hard"};
const Color DIFFICULTY_COLORS[] = {
    { 0x008010 },
    { 0xF08000 },
    { 0xA03000 },
};


Tile::Tile(Texture *tex) : Button(tex) {
    setHidden(false);
    setFlagged(false);
    setMine(false);
    row = 0;
    col = 0;
    isRed = false;
}

enum TileSaveData {
    TILE_HIDDEN = 1,
    TILE_MINE = 2,
    TILE_FLAGGED = 4,
    TILE_RED = 8,
    TILE_REMOVED = 16,

    TILE_DEFAULT = TILE_HIDDEN,
};

Uint8 Tile::save() {
    Uint8 data = 0;
    if (isHidden())  data |= TileSaveData::TILE_HIDDEN;
    if (isMine())    data |= TileSaveData::TILE_MINE;
    if (isFlagged()) data |= TileSaveData::TILE_FLAGGED;
    if (isRed)       data |= TileSaveData::TILE_RED;
    if (removed)     data |= TileSaveData::TILE_REMOVED;
    return data;
}

void Tile::load(Uint8 data) {
    setMine   (data & TileSaveData::TILE_MINE);
    setHidden (data & TileSaveData::TILE_HIDDEN);
    setFlagged(data & TileSaveData::TILE_FLAGGED);
    isRed = data & TileSaveData::TILE_RED;
    removed = data & TileSaveData::TILE_REMOVED;
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

enum TileAnim {
    FLAG_ANIM = 1,
    UNCOVER,
    REVEALMINE,
};


void Tile::playFlagAnim() {
    if (animState.active == TileAnim::FLAG_ANIM) return;

    overlay = nullptr;
    background = &backgrounds[TileBG::HIDDEN];

    animState.start(TileAnim::FLAG_ANIM, new FlagAnim(&overlays[TileOverlay::FLAG], {x, y}, flagged),
        [&overlay=overlay, overlays=overlays, &flagged=flagged] () {
            if (flagged) overlay = &overlays[TileOverlay::FLAG];
        }
    );
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
    animState.start(-1, new WinTileAnim({x, y}, TILE_SIZE), [](){});
    background = nullptr;
    overlay = nullptr;
    removed = true;
}


void Tile::flip(bool recurse, Uint32 delay) {
    setHidden(false);
    if (isMine()) {
        overlay = nullptr;

        animState.start(TileAnim::REVEALMINE, new MineRevealAnim({x,y}, TILE_SIZE), [](){}, delay);
        animState.onstart = [this](){
            overlay = &overlays[TileOverlay::MINE];
            background = &backgrounds[TileBG::BLANK_SQUARE];
        };
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
        animState.start(TileAnim::UNCOVER,
                new UncoverAnim(&backgrounds[TileBG::HIDDEN], {x, y}, game->rng),
                [](){}, delay
        );

        // Use onstart to set textures to account for `delay` parameter
        animState.onstart = [this]() {
            overlay = &numbers[countTouchingMines()];
            background = &backgrounds[TileBG::BLANK_SQUARE];
        };
}

void Tile::foreach_touching_tile(std::function<void(Tile&)> callback, bool diagonals) const {
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
        overlay->render(x + (TILE_SIZE - overlay->getWidth()) / 2, y + (TILE_SIZE - overlay->getHeight()) / 2);
    }
}

void Game::updateFlagCount() {
    int flagCount = 0;
    for (auto& row : board) for (auto& tile : row) {
        flagCount += tile.isFlagged();
    }

    flagCounter.setString(std::to_string(flagCount)
                        + "/"
                        + std::to_string(mineCount)
                        + " flags");

    flagCounter.load();
    flagCounter.x = (screen_width - flagCounter.getWidth()) / 2;
}

TextButton& Game::activeRestartButton() {
    return (state & GameState::OVER) ? playAgainBtn : restartBtn;
}

void Game::OnUpdate(double dt) {
    for (auto& row : board) for (Tile& tile : row) {
        tile.OnUpdate(dt);
    }

    animState.update(dt);

    activeRestartButton().render();

    title.render();

    flagCounter.render();

    for (auto& btn : difficultyBtns) {
        btn.render();
    }

}

Game::~Game() {
    SDL_free(saveDirectory);
}

Game::Game(SDL_Window *window) : Game(window, SIZES[1].rows, SIZES[1].cols) {}

Game::Game(SDL_Window *window, int rows, int cols)
    : rows(rows)
    , cols(cols)
    , mainFont("fonts/Arbutus-Regular.ttf")
    , window(window)
    , title(&mainFont, "Minesweeper")
    , flagCounter(&mainFont, "0/? flags", 0xA00000)
    , restartBtn(&mainFont, "Restart!", 0xFF1000)
    , playAgainBtn(&mainFont, "Play again?", 0x00C000)
{
    currentHover = nullptr;
    rng.seed(std::random_device{}());

    saveDirectory = SDL_GetPrefPath("grassdne", "sdlminesweeper");
    if (saveDirectory == NULL) {
        printf("Error getting save directory: %s", SDL_GetError());
    }

    loadMedia();
}


void Game::OnStart() {
    load();
    int loadedState = state;

    board.resize(rows, std::vector<Tile>(cols));
    ready();

    if (!tileDatas.empty()) {
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
            board[r][c].load(tileDatas[r*cols+c]);
        }
        for (auto &row : board) for (auto& tile : row) {
            tile.forceUpdateTexture();
        }
        updateFlagCount();
    }
    if (loadedState) {
        state = loadedState;
    }
}

// Called on both initial start and restart
void Game::ready() {
    mineCount = rows * cols * PERCENT_MINES;
    animState.kill();

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            Tile &tile = board[row][col];
            tile.row = row;
            tile.col = col;
            tile.setGame(this);
            tile.reset();
        }
    }

    positionItems();

    updateFlagCount();

    state = GameState::READY;
}

namespace Save {
    char HEADER[] = "MINE ";
    const char *FILE = "data.bin";
    size_t DATA_BUFFER = 1028;
}

void Game::save() {
    std::string file = std::string(saveDirectory) + Save::FILE;

    SDL_RWops* out = SDL_RWFromFile(file.c_str(), "w+b");

    SDL_RWwrite(out, Save::HEADER, 1, sizeof(Save::HEADER) - 1);

    SDL_WriteU8(out, 'r');
    SDL_WriteU8(out, (Uint8)rows);
    SDL_WriteU8(out, 'c');
    SDL_WriteU8(out, (Uint8)cols);

    SDL_WriteU8(out, 'g');
    SDL_WriteU8(out, (Uint8)state);

    for (auto& row : board) for (auto& tile : row) {
        SDL_WriteU8(out, 't');
        SDL_WriteU8(out, tile.save());
    }

    SDL_WriteU8(out, '\0');
    SDL_RWclose(out);
}

void Game::load() {
    std::string file = std::string(saveDirectory) + Save::FILE;

    SDL_RWops* in = SDL_RWFromFile(file.c_str(), "r+b");
    if (in == NULL) {
        return;
    }
    for (char *c = Save::HEADER; *c != '\0'; ++c) {
        Uint8 u8 = SDL_ReadU8(in);
        if (u8 != *c) {
            printf("Invalid or corrupted save file! Missing header.\n");
        }
    }
    Uint8 data;
    if ((data = SDL_ReadU8(in)) != 'r') {
        printf("Missing rows data :: expected r (%d) got (%d)\n", 'r', data);
        return;
    }
    rows = SDL_ReadU8(in);

    if (SDL_ReadU8(in) != 'c') {
        printf("Missing cols data\n");
        return;
    }
    cols = SDL_ReadU8(in);

    if (SDL_ReadU8(in) != 'g') {
        printf("Missing game state\n");
        return;
    }
    state = SDL_ReadU8(in);

    tileDatas.resize(rows*cols, TileSaveData::TILE_DEFAULT);
    for (int i = 0; (data = SDL_ReadU8(in)) == 't'; i++) {
        tileDatas[i] = SDL_ReadU8(in);
    }

    SDL_RWclose(in);
}

void Game::resizeBoard() {
    // Resize 2d vector
    board.resize(rows);
    for (auto& row : board) {
        row.resize(cols);
    }

    restartGame();
}

void Game::restartGame() {
    ready();
}

void Game::onMouseButtonDown(SDL_MouseButtonEvent const & e) {
    if (currentHover) {
        // Tile is hovered over
        if (e.button == SDL_BUTTON_LEFT) {
            if (currentHover->isHidden() && currentHover->isUnflagged()) {
                if (state & GameState::STARTED) {
                    currentHover->flip();
                    onRevealTile(*currentHover);
                } else {
                    // Build starting area
                    generateStartingArea(*currentHover);
                    state |= GameState::STARTED;
                }
            }
        } else if (e.button == SDL_BUTTON_RIGHT && currentHover->isHidden()) {
            // Flag tile
            if (currentHover->isUnflagged()) {
                currentHover->flag();
            } else {
                currentHover->unflag();
            }
        }
    }
    else if (e.button == SDL_BUTTON_LEFT) {
        if (activeRestartButton().isMouseOver(e.x, e.y)) {
            return restartGame();
        }
        for (size_t i = 0; i < difficultyBtns.size(); ++i) {
            if (difficultyBtns[i].isMouseOver(e.x, e.y)) {
                rows = SIZES[i].rows;
                cols = SIZES[i].cols;
                resizeBoard();
                return;
            }
        }
    }

}

enum GameAnims {
    EXPLODE = 1,
};

void Game::onLost(Tile& mine) {
    state |= GameState::LOST;

    mine.animState.kill();
    mine.red();

    animState.start(GameAnims::EXPLODE,
                    new DetonationAnim(rng, {mine.x, mine.y}, TILE_SIZE),
                   [this](){
        for (auto& row: board) for (Tile& tile : row) {
                if (tile.isMine() && tile.isHidden() && tile.isUnflagged()) {
                    // Reveal all mines
                    tile.flip();
                }
                else if (tile.isSafe() && tile.isFlagged()) {
                    // Incorrect flag
                    tile.red();
                }
            }
    });

    std::vector<Tile *> mines;
    double delay = 0;
    for (auto& row : board) for (Tile& tile : row) {
        if (tile.isMine() && tile.isUnflagged()) {
            mines.push_back(&tile);
        }
    }
    std::sort(mines.begin(), mines.end(), [&mine](const Tile *a, const Tile *b) {
            const int aX = a->x - mine.x;
            const int aY = a->y - mine.y;

            const int bX = b->x - mine.x;
            const int bY = b->y - mine.y;

            const int aDistSq = aX*aX + aY*aY;
            const int bDistSq = bX*bX + bY*bY;

            return aDistSq < bDistSq;
    });

    const double deltaDelay = 5.0 / mines.size();

    // Skip first item, the detonated mine
    for (auto it = mines.begin() + 1; it != mines.end(); ++it) {
        (*it)->flip(false, (Uint32)(delay*1000));
        delay += deltaDelay;
    }
}

void Game::onWon() {
    state |= GameState::WON;
    
    for (auto& row : board) for (auto& tile : row) {
        if (tile.isMine()) {
            tile.dissapear();
        }
    }
}

void Game::onRevealTile(Tile& revealed) {
    if (revealed.isMine()) {
        onLost(revealed);
    }
    else if (hasWon()) {
        printf("Game won!\n");
        onWon();
    }
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent const & e) {
    (void)e;

}

void Game::onMouseMove(SDL_MouseMotionEvent const& e) {
    if (state & GameState::OVER) {
        if (currentHover) currentHover->mouseLeave();
        currentHover = nullptr;
        return;
    }
    bool overTile = false;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (board[r][c].isMouseOver(e.x, e.y)) {
                overTile = true;

                if (currentHover) currentHover->mouseLeave();
                (currentHover = &board[r][c])->mouseEnter();

                break;
            }
        }
    }
    if (!overTile && currentHover != nullptr) {
        currentHover->mouseLeave();
        currentHover = nullptr;
    }
}

static void pushHiddenNeighbors(Tile& tile, std::vector<Tile*>& tiles, bool diagonals) {
    tile.foreach_touching_tile([&tiles](Tile& neighbor) {
        for (auto tile : tiles) if (tile == &neighbor) return;
        if (neighbor.isHidden()) {
            tiles.push_back(&neighbor);
        }
    }, diagonals);
}

void Game::flipTiles(Tile& root, int count, std::vector<Tile*>& revealqueue) {
    if (count <= 0) return;

    std::vector<Tile*> tiles;
    std::vector<Tile*> tospread;

    // Start by adding all neighbors, including diagonals
    pushHiddenNeighbors(root, tiles, true);

    while (count > 0 && !tiles.empty()) {
        // Select up to 8 random neighbors
        std::shuffle(tiles.begin(), tiles.end(), rng);
        int num = std::min(int(std::min((long unsigned)tiles.size(), 8UL)), count);

        for (int i = 0; i < num; ++i) {
            count -= 1;
            // Force set hidden to not start reveal animation
            tiles[i]->setHidden(false);

            // Add to queue for delayed reveal animtion
            revealqueue.push_back(tiles[i]);

            // Add for hidden neighbors to be in next potential layer
            tospread.push_back(tiles[i]);
        }

        tiles.clear();

        // Build next layer
        for (auto tile : tospread) {
            pushHiddenNeighbors(*tile, tiles, false);
        }
        tospread.clear();
    }
}

void Game::generateStartingArea(Tile& root) {
    std::vector<Tile*> toreveal;

    toreveal.push_back(&root);

    root.setHidden(false);
    root.setMine(false);

    int count = STARTING_SAFE_COUNT;
    flipTiles(root, count, toreveal);

    generateMines();
    Uint32 delay = 0;
    for (auto& tile : toreveal) {
        tile->flip(true, delay);
        delay += 100;
    }

    onRevealTile(root);
}

void Game::generateMines() {
    std::uniform_int_distribution<> randrow(0, rows);
    std::uniform_int_distribution<> randcol(0, cols);
    for (int i = 0; i < mineCount; ++i) {
        int rowPicked = randrow(rng);
        int colPicked = randcol(rng);

        // Find first available tile
        Tile* tile = nullptr;
        for (int _r = 0; _r < rows; ++_r)
        for (int _c = 0; _c < cols; ++_c) {
            // We want to start from the selected position and work our way around
            // as if it's a circular array
            int r = (_r + rowPicked) % rows;
            int c = (_c + colPicked) % cols;

            if (board[r][c].isHidden() && board[r][c].isSafe()) {
                tile = &board[r][c];

                // Escape from nested loop
                goto found;
            }
        }

        // NO FREE TILES:
        {
            mineCount = i;
            break;
        }


    found:
        // Found a free tile
        tile->setMine(true);

    }

    updateFlagCount();
}

std::string TILE_FILES[] = {
    /*[TileBG::BLANK_SQUARE] = */"images/square_blank.png",
    /*[TileBG::HIDDEN] = */"images/tile.png",
    /*[TileBG::HIGHLIGHT] = */"images/hovered_tile.png",
    /*[TileBG::RED_SQUARE] = */"images/square_red.png",
};

std::string ICON_FILES[] = {
    /*[TileOverlay::FLAG] = */"images/flag.png",
    /*[TileOverlay::MINE] = */"images/mine.png",
};


const Color NUMBER_COLORS[] = {
    0x000000, // Number 0 has no text!
    0x1300d8,
    0x02850e,
    0xcb001e,
    0x130e46,
    0x003e14,
    0x460202,
    0x986207,
    0x7100c7,
};

Texture Tile::backgrounds[TILE_BG_COUNT];
Texture Tile::overlays[TILE_OVERLAY_COUNT];
Texture Tile::numbers[1 + NUMBER_TILES_COUNT];

constexpr float NUMBER_SCALE = 0.8;


void Tile::loadMedia(Font const& font) {
    int w = TILE_SIZE;
    int h = TILE_SIZE;
    for (int i = 0; i < TILE_BG_COUNT; ++i) {
        Tile::backgrounds[i].loadFile(TILE_FILES[i], w, h);
    }

    for (int i = 0; i < TILE_OVERLAY_COUNT; ++i) {
        overlays[i].loadFile(ICON_FILES[i], w, h);
    }
    overlays[TileOverlay::MINE].setMultColor(0.0, 0.0, 0.0);

    for (int i = 1; i <= COUNT_TILE_NUMBERS; ++i) {
        const char num[] = {char(i + '0'), '\0'};

        const Color color = NUMBER_COLORS[i];
        numbers[i].loadText(font.raw(), num, color.as_sdl());
        numbers[i].setScale(NUMBER_SCALE);
    }

}

void Game::loadMedia() {
    title.load();

    flagCounter.setScale(0.4);
    updateFlagCount();

    Tile::loadMedia(mainFont);

    restartBtn.setScale(0.5);
    restartBtn.load();

    playAgainBtn.setScale(0.5);
    playAgainBtn.load();

    {

        constexpr int NUMBTNS = sizeof(SIZES) / sizeof(SIZES[0]);
        difficultyBtns.resize(NUMBTNS, &mainFont);

        for (int i = 0; i < NUMBTNS; ++i) {
            auto& btn = difficultyBtns[i];
            btn.text.setColor(DIFFICULTY_COLORS[i]);
            btn.text.setString(DIFFICULTY_STRINGS[i]);
            btn.text.load();
            btn.setScale(0.4);
        }

    }
}

void Game::positionItems() {
    SDL_GetWindowSize(window, &screen_width, &screen_height);

    int y = 0;

    // Title
    y += TITLE_SPACE_ABOVE;
    title.x = (screen_width - title.getWidth()) / 2; 
    title.y = y;
    y += title.getHeight() + 20;

    // Flag count text
    flagCounter.x = (screen_width - flagCounter.getWidth()) / 2;
    flagCounter.y = y;
    y += flagCounter.getHeight() + 20;

    // Tiles
    int x = (screen_width - cols*TILE_SIZE) / 2;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            board[row][col].x = x + col * TILE_SIZE;
            board[row][col].y = y + row * TILE_SIZE;
        }
    }
    y += rows * TILE_SIZE;

    // Play again and restart buttons
    y += TILE_SIZE;
    restartBtn.text.setColor(Color(0xFF1000));
    restartBtn.text.setString("Restart!");
    restartBtn.setScale(0.5);
    restartBtn.load();
    restartBtn.setCenterX(screen_width / 2);
    restartBtn.setY(y);

    playAgainBtn.text.setColor(Color(0x00C000));
    playAgainBtn.text.setString("Play again?");
    playAgainBtn.setScale(0.5);
    playAgainBtn.load();
    playAgainBtn.setCenterX(screen_width / 2);
    playAgainBtn.setY(y);

    y += playAgainBtn.getHeight() + 20;

    for (auto& btn : difficultyBtns) btn.setY(y);


    difficultyBtns[1].setCenterX(screen_width / 2);

    int midWidth = difficultyBtns[1].getWidth();

    difficultyBtns[0].setCenterX((screen_width - midWidth - difficultyBtns[0].getWidth()) / 2 - 10);

    difficultyBtns[2].setCenterX((screen_width + midWidth + difficultyBtns[2].getWidth()) / 2 + 10);

    y += difficultyBtns[0].getHeight() + 20;

    if (y > screen_height) {
        SDL_SetWindowSize(window, screen_width, y);
    }
}

bool Game::hasWon() {
    for (auto& row : board) for (auto& tile : row) {
        if (tile.isHidden() && tile.isSafe()) {
            return false;
        }
    }
    // All tiles that aren't mines have been revealed!
    return true;
}



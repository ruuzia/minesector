#include "common.h"
#include "game.h"
#include <string>
#include <set>
#include <algorithm>

const int TITLE_FONT_SIZE = 40;
const int TITLE_SPACE_ABOVE = 15;
const int TILE_SIZE = 32;
const float PERCENT_MINES = 0.20;

const struct {int cols; int rows; } SIZES[] = { {10, 8}, {12, 10}, {14, 12} };
const std::string DIFFICULTY_STRINGS[] = {"Easy", "Medium", "Hard"};
const SDL_Color DIFFICULTY_COLORS[] = {
    {0x00,0x80,0x10,0xFF},
    {0xF0,0x80,0x00,0xFF},
    {0xA0,0x30,0x00,0xFF},
};


Tile::Tile(Texture *tex) : Button(tex) {
    setHidden(false);
    setFlagged(false);
    setMine(false);
    row = 0;
    col = 0;
    isRed = false;
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
    FLAG = 1,
    UNCOVER,
    REVEALMINE,
};


void Tile::playFlagAnim() {
    if (animState.active == TileAnim::FLAG) return;

    overlay = nullptr;
    background = &backgrounds[TTEX_HIDDEN];

    animState.start(TileAnim::FLAG, new FlagAnim(&overlays[ICON_FLAG], {x, y}, flagged),
        [&overlay=overlay, overlays=overlays, &flagged=flagged] () {
            if (flagged) overlay = &overlays[ICON_FLAG];
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
        background = &backgrounds[TTEX_HIGHLIGHT];
    }
}

void Tile::mouseLeave() {
    if (background == &backgrounds[TTEX_HIGHLIGHT]) {
        background = &backgrounds[TTEX_HIDDEN];
    }
}

void Tile::reset() {
    animState.kill();
    setMine(false);
    setFlagged(false);
    setHidden(true);
    background = &backgrounds[TTEX_HIDDEN];
    overlay = nullptr;
    isRed = false;
}

void Tile::red() {
    isRed = true;
    background = &backgrounds[TTEX_RED_SQUARE];
}


void Tile::flip(bool recurse, Uint32 delay) {
    setHidden(false);
    if (isMine()) {
        overlay = nullptr;

        animState.start(TileAnim::REVEALMINE, new MineRevealAnim({x,y}, TILE_SIZE), [](){}, delay);
        animState.onstart = [this](){
            overlay = &overlays[ICON_MINE];
            background = &backgrounds[TTEX_BLANK_SQUARE];
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
                new UncoverAnim(&backgrounds[TTEX_HIDDEN], {x, y}, game->rng),
                [](){}, delay
        );

        // Use onstart to set textures to account for `delay` parameter
        animState.onstart = [this]() {
            overlay = &numbers[countTouchingMines()];
            background = &backgrounds[TTEX_BLANK_SQUARE];
        };
}

void Tile::foreach_touching_tile(std::function<void(Tile&)> callback, bool diagonals) const {
    //printf("row: %d ;; col: %d\n", row, col);
    //printf("rows: %d ;; cols: %d\n\n", game->rows, game->cols);
    fflush(stdout);
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
    // Should always have a background
    SDL_assert(background != nullptr);
    background->render(x, y);

    if (overlay != nullptr) {
        overlay->render(x, y);
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
    TTF_CloseFont(mainFont);
}

Game::Game() : Game(SIZES[1].rows, SIZES[1].cols) {}

Game::Game(int rows, int cols) : rows(rows), cols(cols) {
    mainFont = nullptr;
    currentHover = nullptr;
    state = GameState::READY;
    rng.seed(std::random_device{}());

    board.resize(rows, std::vector<Tile>(cols));
}

void Game::OnStart() {
    ready();
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
    printf("Game lost!\n"); fflush(stdout);

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

void Game::onRevealTile(Tile& revealed) {
    if (revealed.isMine()) {
        onLost(revealed);
    }
    else if (hasWon()) {
        printf("Game won!\n");
        state |= GameState::WON;
    }
    else return;

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
        int num = std::min(int(std::min(tiles.size(), 8UL)), count);

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

    int count = 20;
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
    /*[TTEX_BLANK_SQUARE] = */"images/square_blank.png",
    /*[TTEX_HIDDEN] = */"images/tile.png",
    /*[TTEX_HIGHLIGHT] = */"images/hovered_tile.png",
    /*[TTEX_RED_SQUARE] = */"images/square_red.png",
};

std::string ICON_FILES[] = {
    /*[ICON_FLAG] = */"images/flag.png",
    /*[ICON_MINE] = */"images/mine.png",
};

const char* NUMBER_FILES[] = {
    nullptr,
    "images/number_1.png",
    "images/number_2.png",
    "images/number_3.png",
    "images/number_4.png",
    "images/number_5.png",
    "images/number_6.png",
    "images/number_7.png",
    "images/number_8.png",
};

Texture Tile::backgrounds[COUNT_TTEX];
Texture Tile::overlays[COUNT_ICONS];
Texture Tile::numbers[NUMBER_TILES_COUNT];

void Tile::loadMedia() {
    int w = TILE_SIZE;
    int h = TILE_SIZE;
    for (int i = 0; i < COUNT_TTEX; ++i) {
        Tile::backgrounds[i].loadFile(TILE_FILES[i], w, h);
    }

    for (int i = 0; i < COUNT_ICONS; ++i) {
        overlays[i].loadFile(ICON_FILES[i], w, h);
    }

    for (int i = 1; i < COUNT_TILE_NUMBERS; ++i) {
        if (NUMBER_FILES[i]) {
            std::string file = NUMBER_FILES[i];
            numbers[i].loadFile(file, w, h);
        }
    }
}

void Game::loadMedia(SDL_Window *win) {
    window = win;
    /** Fonts **/
    mainFont = TTF_OpenFont("fonts/Arbutus-Regular.ttf", TITLE_FONT_SIZE);
    if (mainFont == nullptr) {
        throw std::runtime_error("Failed to load font! SDL_ttf error: " + std::string(TTF_GetError()));
    }

    title.setString("Minesweeper");
    title.setColor(0x00, 0x00, 0x00, 0xFF);
    title.setFont(mainFont);
    title.load();

    flagCounter.setColor(0xA0, 0x00, 0x00, 0xFF);
    flagCounter.setFont(mainFont);
    flagCounter.setScale(0.4);
    updateFlagCount();

    Tile::loadMedia();

    restartBtn.text.setFont(mainFont);
    restartBtn.text.setColor(0xFF, 0x10, 0x00);
    restartBtn.text.setString("Restart!");
    restartBtn.setScale(0.5);
    restartBtn.load();

    playAgainBtn.text.setFont(mainFont);
    playAgainBtn.text.setColor(0x00, 0xC0, 0x00);
    playAgainBtn.text.setString("Play again?");
    playAgainBtn.setScale(0.5);
    playAgainBtn.load();

    {

        const int NUMBTNS = sizeof(SIZES) / sizeof(SIZES[0]);
        difficultyBtns.resize(NUMBTNS);

        for (int i = 0; i < NUMBTNS; ++i) {
            auto& btn = difficultyBtns[i];
            btn.text.setFont(mainFont);
            btn.text.setColor(DIFFICULTY_COLORS[i].r, DIFFICULTY_COLORS[i].g, DIFFICULTY_COLORS[i].b);
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
    restartBtn.text.setFont(mainFont);
    restartBtn.text.setColor(0xFF, 0x10, 0x00);
    restartBtn.text.setString("Restart!");
    restartBtn.setScale(0.5);
    restartBtn.load();
    restartBtn.setCenterX(screen_width / 2);
    restartBtn.setY(y);

    playAgainBtn.text.setFont(mainFont);
    playAgainBtn.text.setColor(0x00, 0xC0, 0x00);
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



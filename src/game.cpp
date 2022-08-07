#include "common.h"
#include "game.h"

#include <algorithm>

const int TITLE_FONT_SIZE = 40;
const int TITLE_SPACE_ABOVE = 15;
const int TITLE_SPACE_BELOW = 45;
const int TILE_SIZE = 32;
const float PERCENT_MINES = 0.20;


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
    //background = &backgrounds[TTEX_HIDDEN];

    animState.start(TileAnim::FLAG, new FlagAnim(&overlays[ICON_FLAG], {x, y}, flagged),
        [&overlay=overlay, overlays=overlays, &flagged=flagged] () {
            if (flagged) overlay = &overlays[ICON_FLAG];
        }
    );
}

void Tile::flag() {
    setFlagged(true);
    playFlagAnim();
}

void Tile::unflag() {
    setFlagged(false);
    playFlagAnim();
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

        animState.start(TileAnim::REVEALMINE, new MineRevealAnim({x,y}, TILE_SIZE), [this](){
                overlay = &overlays[ICON_MINE];
        }, delay);

        animState.onstart = [this](){
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
    const int left = col - 1;
    const int right = col + 1;
    const int below = row + 1;
    const int above = row - 1;

    const bool spaceLeft = left >= 0;
    const bool spaceRight = right < game->cols;
    const bool spaceAbove = above >= 0;
    const bool spaceBelow = below < game->cols;

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
    if (background) {
        background->render(x, y);
    }
    if (overlay) {
        overlay->render(x, y);
    }
}

void Game::OnStart() {
}

void Game::OnUpdate(double dt) {
    int screen_width;
    SDL_GetWindowSize(window, &screen_width, nullptr);

    /** textures.title **/
    int x_title = screen_width / 2 - textures.title.getWidth() / 2; 
    textures.title.render(x_title, TITLE_SPACE_ABOVE);

    for (auto& row : board) for (Tile& tile : row) {
        tile.OnUpdate(dt);
    }

    animState.update(dt);

    if (state & GameState::OVER) {
        playAgainBtn.render();
    }
}

Game::Game(int rows, int cols) : rows(rows), cols(cols) {
    window = nullptr;
    renderer = nullptr;
    mainFont = nullptr;
    currentHover = nullptr;
    board.resize(rows, std::vector<Tile>(cols));
    state = GameState::READY;

    rng.seed(std::random_device{}());

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            Tile& tile = board[row][col];
            tile.row = row;
            tile.col = col;
            tile.setHidden(true);
            tile.setGame(this);
        }
    }
}

Game::~Game() {
    TTF_CloseFont(mainFont);
    textures.title.free();

}

void Game::restartGame() {
    state = GameState::READY;
    animState.kill();
    for (auto& row:board) for (auto& tile : row) {
        tile.reset();
    }
}

void Game::onMouseButtonDown(SDL_MouseButtonEvent const & e) {
    if (state & GameState::OVER) {
        if (e.button == SDL_BUTTON_LEFT && playAgainBtn.isMouseOver(e.x, e.y)) {
            restartGame();
        }
        return;
    }
    if (currentHover == nullptr) return;

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

enum GameAnims {
    EXPLODE = 1,
};

void Game::onLost(Tile& mine) {
    printf("Game lost!\n"); fflush(stdout);

    state |= GameState::LOST;

    mine.animState.kill();
    mine.red();

    animState.start(GameAnims::EXPLODE,
                    new DetonationAnim(rng, {mine.x, mine.y}, &textures.detonationParticle),
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

void Game::flipTiles(Tile& root, int& count, std::vector<Tile*>& toreveal, bool diagonals) {
    if (count <= 0) return;

    std::vector<Tile *> touching(8);
    //std::vector<Tile *> out(touching.size());
    std::vector<Tile *> recurse(touching.size());

    root.foreach_touching_tile([&touching](Tile& tile) {
        touching.push_back(&tile);
    }, diagonals);

    //std::sample(touching.begin(), touching.end(), std::back_inserter(out), touching.size(), rng);
    std::shuffle(touching.begin(), touching.end(), rng);
    
    for (auto tile : touching) {
        if (tile == nullptr) continue;
        if (tile->isRevealed()) continue;

        count -= 1;

        tile->setHidden(false);
        toreveal.push_back(tile);
        recurse.push_back(tile);
    }
    if (count <= 0) return;

    for (auto tile : recurse) {
        if (tile == nullptr) continue;
        flipTiles(*tile, count, toreveal, false);
    }

}

void Game::generateStartingArea(Tile& root) {
    std::vector<Tile*> toreveal;

    toreveal.push_back(&root);

    root.setHidden(false);
    root.setMine(false);

    int count = 20;
    int picknum = 4;
    flipTiles(root, count, toreveal, picknum);

    generateMines();
    Uint32 delay = 0;
    for (auto& tile : toreveal) {
        tile->flip(true, delay);
        delay += 100;
    }
}

void Game::generateMines() {
    int count = rows * cols * PERCENT_MINES;
    std::uniform_int_distribution<> randtile(0, rows * cols - 1);
    while (count > 0) {
        int n = randtile(rng);
        int row = n / cols;
        int col = n % cols;

        Tile& tile = board[row][col];
        if (tile.isHidden() && tile.isSafe()) {
            tile.setMine(true);
            --count;
        }
    }
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
    // Initialize member variables
    window = win;
    renderer = SDL_GetRenderer(win);

    int screen_width;
    SDL_GetWindowSize(window, &screen_width, nullptr);

    /** Fonts **/
    mainFont = TTF_OpenFont("fonts/Arbutus-Regular.ttf", TITLE_FONT_SIZE);
    if (mainFont == nullptr) {
        throw std::runtime_error("Failed to load font! SDL_ttf error: " + std::string(TTF_GetError()));
    }

    { /** Title texture **/
        SDL_Color titleColor = { 0, 0, 0, 0 };
        textures.title.loadText(mainFont, "Minesweeper", titleColor);
    }

    Tile::loadMedia();

    int top = TITLE_SPACE_ABOVE + textures.title.getHeight() + TITLE_SPACE_BELOW;
    { /** Tile buttons **/
        int left = (screen_width - cols*TILE_SIZE) / 2;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                Tile &tile = board[row][col];
                tile.x = left + col * TILE_SIZE;
                tile.y = top + row * TILE_SIZE;
                tile.reset();
            }
        }
    }

    std::string f = "images/detonationParticle.png";
    textures.detonationParticle.loadFile(f, 32, 32);

    { /** Play Again Button **/
        playAgainBtn.setColor(0x00, 0xC0, 0x00);
        playAgainBtn.setFont(mainFont);
        playAgainBtn.setString("Play again?");
        playAgainBtn.scale = 0.5;
        playAgainBtn.load();
        playAgainBtn.setCenterPos(screen_width / 2, rows * TILE_SIZE + top + 50);
        playAgainBtn.x = (screen_width - playAgainBtn.getWidth()) / 2;
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



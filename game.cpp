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

void Tile::onClick() { }

void Tile::reveal() {
    setHidden(false);
    updateTexture();
    if (isEmpty() && countTouchingMines() == 0) {
        // Recursively reveal surrounding tiles
        foreach_touching_tile([](Tile& tile) {
            if (tile.isHidden()) {
                tile.reveal();
            }
        });
    }
}

int Tile::countTouchingMines() {
    int nearbyMines = 0;

    foreach_touching_tile([&](Tile& tile) -> void {
        if (tile.isMine()) {
            nearbyMines += 1;
        }
    });

    return nearbyMines;
}


void Tile::updateTexture(bool mouseOver) {
    if (isHidden() && isFlagged()) {
        texture = &game->textures.tiles[TTEX_HIDDEN];
        overlay = &game->textures.icons[ICON_FLAG];
    }
    
    else if (isHidden() && mouseOver) {
        texture = &game->textures.tiles[TTEX_HIGHLIGHT];
        overlay = nullptr;
    }
    
    else if (isHidden()) {
        texture = &game->textures.tiles[TTEX_HIDDEN];
        overlay = nullptr;
    }
    
    else if (isMine()) {
        texture = &game->textures.tiles[isRed ? TTEX_RED_SQUARE : TTEX_BLANK_SQUARE];
        overlay = &game->textures.icons[ICON_MINE];
    }
    
    else {
        texture = &game->textures.tiles[isRed ? TTEX_RED_SQUARE : TTEX_BLANK_SQUARE];
        overlay = &game->textures.numbers[countTouchingMines()];
    }
}

void Tile::foreach_touching_tile(std::function<void(Tile&)> callback, bool diagonals) {
    const bool spaceLeft = col > 0;
    const bool spaceRight = col < game->cols - 1;
    const bool spaceAbove = row > 0;
    const bool spaceBelow = row < game->rows - 1;
    const int left = col - 1;
    const int right = col + 1;
    const int below = row + 1;
    const int above = row - 1;

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

void Tile::render(SDL_Renderer *renderer) {
    if (texture) {
        texture->render(renderer, x, y);
    }
    if (overlay) {
        overlay->render(renderer, x, y);
    }
}

void Game::OnStart() {
}

void Game::OnUpdate(int dt) {
    int screen_width;
    SDL_GetWindowSize(window, &screen_width, NULL);

    /** textures.title **/
    int x_title = screen_width / 2 - textures.title.getWidth() / 2; 
    textures.title.render(renderer, x_title, TITLE_SPACE_ABOVE);

    for (auto& row : board) for (Tile& tile : row) {
        tile.render(renderer);
    }
}

Game::Game(int rows, int cols) : rows(rows), cols(cols) {
    window = nullptr;
    renderer = nullptr;
    mainFont = nullptr;
    currentHover = nullptr;
    board.resize(rows, std::vector<Tile>(cols));
    state = GAME_READY;

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

void Game::onMouseButtonDown(SDL_MouseButtonEvent &e) {
    if (currentHover == nullptr || state & GAME_OVER) return;

    if (e.button == SDL_BUTTON_LEFT) {
        if (currentHover->isHidden() && currentHover->isUnflagged()) {
            if (state & GAME_STARTED) {
                // Reveal tile
                currentHover->reveal();
                onRevealTile(*currentHover);
            } else {
                // Build starting area
                generateStartingArea(*currentHover);
                state |= GAME_STARTED;
            }
        }
    } else if (e.button == SDL_BUTTON_RIGHT && currentHover->isHidden()) {
        // Flag tile
        currentHover->setFlagged(currentHover->isUnflagged());
    }

    currentHover->updateTexture();
}

void Game::onRevealTile(Tile& revealed) {
    if (revealed.isMine()) {
        printf("Game lost!\n");
        state |= GAME_LOST;
        revealed.reveal();
        revealed.isRed = true;

        for (auto& row: board) for (Tile& tile : row) {
            if (tile.isMine() && tile.isHidden() && tile.isUnflagged()) {
                tile.reveal();
            }
        }
    } else if (hasWon()) {
        printf("Game won!\n");
        state |= GAME_WON;
    }
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent &e) {

}

void Game::onMouseMove(SDL_Event *e, int x, int y) {
    if (state & GAME_OVER) {
        if (currentHover) currentHover->updateTexture(false);
        currentHover = nullptr;
        return;
    }
    bool overTile = false;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (board[r][c].isMouseOver(x, y)) {
                overTile = true;
                if (currentHover) currentHover->updateTexture(false);
                currentHover = &board[r][c];
                currentHover->updateTexture(true);
                break;
            }
        }
    }
    if (!overTile && currentHover != nullptr) {
        currentHover->updateTexture(false);
        currentHover = nullptr;
    }
}

void Game::flipTiles(Tile& root, int& count, bool diagonals) {
    if (count <= 0) return;

    std::vector<Tile *> touching(8);
    std::vector<Tile *> out(touching.size());
    std::vector<Tile *> recurse(touching.size());

    root.foreach_touching_tile([&count, &touching](Tile& tile) {
        touching.push_back(&tile);
    }, diagonals);

    std::sample(touching.begin(), touching.end(), std::back_inserter(out), touching.size(), rng);
    
    for (auto tile : out) {
        if (tile == nullptr) continue;
        if (tile->isRevealed()) continue;

        count -= 1;

        tile->setHidden(false);
        tile->setMine(false);
        recurse.push_back(tile);
    }
    if (count <= 0) return;

    for (auto tile : recurse) {
        if (tile == nullptr) continue;
        flipTiles(*tile, count, false);
    }

}

void Game::generateStartingArea(Tile& root) {
    root.setHidden(false);
    root.setMine(false);
    int count = 20;
    int picknum = 4;
    flipTiles(root, count, picknum);

    generateMines();

    for (auto& row : board) for (auto& tile : row) {
        if (tile.isRevealed()) {
            tile.reveal();
        }
        else {
            tile.updateTexture();
        }
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
        if (tile.isHidden() && tile.isEmpty()) {
            tile.setMine(true);
            --count;
        }
    }
}

const char* TILE_FILES[] = {
    "images/square_blank.png", /* TTEX_NUMBER_0 */ 
    "images/tile.png",         /* TTEX_HIDDEN */
    "images/hovered_tile.png", /* TTEX_HIGHLIGHT */
    "images/square_red.png",   /* TTEX_RED_SQUARE */
};

const char* ICON_FILES[] = {
    "images/flag.png", /* TTEX_FLAG */
    "images/mine.png",    /* TTEX_MINE */
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


bool Game::loadMedia(SDL_Window *win) {
    // Initialize member variables
    window = win;
    renderer = SDL_GetRenderer(win);

    int screen_width;
    SDL_GetWindowSize(window, &screen_width, NULL);

    /** Fonts **/
    mainFont = TTF_OpenFont("fonts/Arbutus-Regular.ttf", TITLE_FONT_SIZE);
    if (mainFont == NULL) {
        printf("Failed to load font! SDL_ttf error: %s\n", TTF_GetError());
        return FAIL;
    }

    { /** Title texture **/
        SDL_Color titleColor = { 0, 0, 0 };
        if (textures.title.loadText(renderer, mainFont, "Minesweeper", titleColor) == FAIL) {
            return FAIL;
        }
    }

    { /** Tile Textures **/
        int w = TILE_SIZE;
        int h = TILE_SIZE;
        for (int i = 0; i < COUNT_TTEX; ++i) {
            if (textures.tiles[i].loadFile(renderer, TILE_FILES[i], w, h) == FAIL)
                return FAIL;
        }

        for (int i = 0; i < COUNT_ICONS; ++i) {
            if (textures.icons[i].loadFile(renderer, ICON_FILES[i], w, h) == FAIL)
                return FAIL;
        }

        for (int i = 0; i < COUNT_TILE_NUMBERS; ++i) {
            if (NUMBER_FILES[i]) {
                if (textures.numbers[i].loadFile(renderer, NUMBER_FILES[i], w, h) == FAIL)
                    return FAIL;
            }
        }
    }

    { /** Tile buttons **/
        int top = TITLE_SPACE_ABOVE + textures.title.getHeight() + TITLE_SPACE_BELOW;
        int left = (screen_width - cols*TILE_SIZE) / 2;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                Tile &tile = board[row][col];
                tile.x = left + col * TILE_SIZE;
                tile.y = top + row * TILE_SIZE;
                tile.updateTexture();
            }
        }
    }

    return OK;
}

bool Game::hasWon() {
    for (auto& row : board) for (auto& tile : row) {
        if (tile.isRevealed() && tile.isEmpty()) {
            return false;
        }
    }
    return true;
}



#include "common.h"
#include "game.h"

Tile::Tile(Texture *tex) : Button(tex) {
    isHidden = false;
    isFlagged = false;
    isMine = false;
    row = 0;
    col = 0;
}

void Tile::onClick() {
    printf("I've been clicked!\n");
}

void Tile::updateTexture(bool mouseOver) {
    TileTexture tex;

    if (isHidden && isFlagged) {
        tex = TTEX_FLAG;
    }
    else if (isHidden && mouseOver) {
        tex = TTEX_HIGHLIGHT;
    }
    else if (isHidden) {
        tex = TTEX_HIDDEN;
    }
    else if (isMine) {
        tex = TTEX_MINE;
    }
    else {
        tex = TTEX_NUMBER_0;
    }
    texture = &game->textures.tile[tex];
}

void Tile::foreach_touching_tile(std::function<void(Tile&)> callback) {
    const bool spaceLeft = col > 0;
    const bool spaceRight = col < game->cols - 1;
    const bool spaceAbove = row > 0;
    const bool spaceBelow = row < game->rows - 1;
    const int left = col - 1;
    const int right = col + 1;
    const int below = row + 1;
    const int above = row - 1;

    auto& tiles = game->tiles;

    if (spaceLeft) callback(tiles[row][left]);
    if (spaceRight) callback(tiles[row][right]);
    if (spaceAbove) callback(tiles[above][col]);
    if (spaceBelow) callback(tiles[below][col]);
    if (spaceLeft && spaceAbove) callback(tiles[above][left]);
    if (spaceRight && spaceAbove) callback(tiles[above][right]);
    if (spaceLeft && spaceBelow) callback(tiles[below][left]);
    if (spaceRight && spaceBelow) callback(tiles[below][right]);
}

void Game::OnStart() {
    hoverTexture = &textures.tile[TTEX_HIGHLIGHT];
}

void Game::OnUpdate(int dt) {
    int screen_width;
    SDL_GetWindowSize(window, &screen_width, NULL);

    /** textures.title **/
    int x_title = screen_width / 2 - textures.title.getWidth() / 2; 
    textures.title.render(renderer, x_title, TITLE_SPACE_ABOVE);

    for (auto& row : tiles) for (Tile& tile : row) {
        tile.render(renderer);
    }
}

Game::Game(int rows, int cols) : rows(rows), cols(cols) {
    window = nullptr;
    renderer = nullptr;
    mainFont = nullptr;
    currentHover = nullptr;
    hoverTexture = nullptr;
    tiles.resize(rows, std::vector<Tile>(cols));
    started = false;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            Tile& tile = tiles[row][col];
            tile.row = row;
            tile.col = col;
            tile.isHidden = true;
            tile.setGame(this);
        }
    }
}

Game::~Game() {
    TTF_CloseFont(mainFont);
    textures.title.free();

    rng.seed(time(0));
}

void Game::onMouseButtonDown(SDL_MouseButtonEvent &e) {
    if (currentHover != nullptr and currentHover->texture == hoverTexture && e.button == SDL_BUTTON_LEFT) {
        hoverTexture = &textures.tile[TTEX_CLICKING];
        currentHover->texture = &textures.tile[TTEX_CLICKING];
    }
}
void Game::onMouseButtonUp(SDL_MouseButtonEvent &e) {
    if (currentHover == nullptr) return;

    if (e.button == SDL_BUTTON_LEFT) {
        if (currentHover->texture == hoverTexture) {
            if (started) {
                // Reveal tile
                currentHover->isHidden = false;
                if (currentHover->isMine) {
                    currentHover->texture = &textures.tile[TTEX_MINE];
                } else {
                    currentHover->texture = &textures.tile[countTouchingMines(*currentHover)];
                }
            } else {
                // Build starting area
                generateStartingArea(*currentHover);
                started = true;
            }
            hoverTexture = &textures.tile[TTEX_HIGHLIGHT];

        } else if (currentHover->isFlagged) {
            // Unflag tile
            currentHover->isFlagged = false;
            currentHover->texture = &textures.tile[TTEX_HIDDEN];
        }
    } else if (e.button == SDL_BUTTON_RIGHT && currentHover->isHidden) {
        // Flag tile
        currentHover->isFlagged = true;
        currentHover->texture = &textures.tile[TTEX_FLAG];
    }
}

void Game::onMouseMove(SDL_Event *e, int x, int y) {
    bool overTile = false;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (tiles[r][c].isMouseOver(x, y)) {
                overTile = true;
                if (currentHover != nullptr && currentHover->texture == hoverTexture) {
                    currentHover->texture = &textures.tile[TTEX_HIDDEN];
                }
                currentHover = &tiles[r][c];
                if (currentHover->texture == &textures.tile[TTEX_HIDDEN]) {
                    currentHover->texture = hoverTexture;
                }
                break;
            }
        }
    }
    if (!overTile && currentHover != nullptr) {
        if (currentHover->texture == hoverTexture) {
            currentHover->texture = &textures.tile[TTEX_HIDDEN];
        }
        currentHover = nullptr;
    }
}

int Game::countTouchingMines(Tile& tile) {
    int nearbyMines = 0;

    tile.foreach_touching_tile([&nearbyMines](Tile& tile) {
        if (tile.isMine) {
            nearbyMines += 1;
        }
    });

    return nearbyMines;
}

void Game::generateStartingArea(Tile& tile) {
    tile.isHidden = false;
    tile.isMine = false;
    tile.foreach_touching_tile([](Tile& tile) {
        tile.isHidden = false;
        tile.isMine = false;
    });

    generateMines();

    for (auto& row : tiles) for (auto& tile : row) {
        if (!tile.isHidden) {
            // Not a mine bc mines are all hidden
            tile.texture = &textures.tile[countTouchingMines(tile)];
        }
    }
}

void Game::generateMines() {
    int count = rows * cols * MINE_RATIO;
    std::uniform_int_distribution<> randtile(0, rows * cols - 1);
    while (count > 0) {
        int n = randtile(rng);
        int row = n / cols;
        int col = n % rows;

        Tile& tile = tiles[row][col];
        if (tile.isHidden && !tile.isMine) {
            tile.isMine = true;
            --count;
        }
    }
}

const char* TILE_FILES[] = {
    "images/empty.png",        /* TTEX_NUMBER_0 */ 

    "images/number_1.png",
    "images/number_2.png",
    "images/number_3.png",
    "images/number_4.png",
    "images/number_5.png",
    "images/number_6.png",
    "images/number_7.png",
    "images/number_8.png",     /*TTEX_NUMBER_8*/
   
    "images/tile.png",         /* TTEX_HIDDEN */
    "images/hovered_tile.png", /* TTEX_HIGHLIGHT */
    "images/clicked_tile.png", /* TTTTEX_CLICKING */
    "images/tile_flagged.png", /* TTTTEX_FLAG */
    "images/tile_mine.png",    /* TTTTEX_MINE */
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
        for (int i = TTEX_NUMBER_0; i < COUNT_TTEX; ++i) {
            if (textures.tile[i].loadFile(renderer, TILE_FILES[i], w, h) == FAIL)
                return FAIL;
        }
    }

    { /** Tile buttons **/
        int top = TITLE_SPACE_ABOVE + textures.title.getHeight() + TITLE_SPACE_BELOW;
        int left = (screen_width - cols*TILE_SIZE) / 2;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                Tile &tile = tiles[row][col];
                tile.texture = &textures.tile[TTEX_HIDDEN];
                tile.x = left + col * TILE_SIZE;
                tile.y = top + row * TILE_SIZE;
            }
        }
    }

    return OK;
}



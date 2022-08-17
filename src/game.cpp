#include <string>
#include <algorithm>
#include "common.h"
#include "game.h"
#include "color.h"

namespace Detonation {
    namespace Particle {
        namespace Speed {
            constexpr float MAX = 150.0;
            constexpr float MIN = -MAX;
        }
        
        namespace Green {
            constexpr float MIN = 0.0;
            constexpr float MAX = 0.6;
        }

        namespace Lifetime {
            constexpr float MIN = 1.0;
            constexpr float MAX = 2.0;
        }

        namespace Depth {
            constexpr int MIN = 0;
            constexpr int MAX = 2;
        }
        constexpr float POS_VARIATION = 100.0;

        namespace Ember {
            constexpr float DELTA_ALPHA = -0.2;

            namespace Size {
                constexpr float MIN = 6;
                constexpr float MAX = 10;
            }
        }

        namespace Piece {
            constexpr float DELTA_ALPHA = -0.1;

            namespace Size {
                constexpr float MAX = 15;
            }
        }
    }

    namespace Emitter {
        constexpr float PERIOD = 0.05;
        constexpr int COUNT = 3;
        constexpr float TIME = 10.0;
    }
}

std::string SOUND_FILES[SoundEffects::COUNT] {
    "sounds/flag.wav",
    "sounds/whoosh.wav",
    "sounds/blip.wav",
    "sounds/explode.wav",
    "sounds/shovel.wav",
};

constexpr int VERT_COUNT = 6;


class DetonationParticle {
protected:
    double lifetime;
    double dx;
    double dy;

    float x;
    float y;
    
    double born;

public:
    int depth;
    DetonationParticle(std::mt19937& rng, float x, float y) : x(x), y(y) {

        born = SECONDS();
        using namespace Detonation::Particle;
        depth = std::uniform_int_distribution<>(Depth::MIN, Depth::MAX) (rng);
    }
    virtual ~DetonationParticle() = default;

    virtual void render(double dt) = 0;

    [[nodiscard]] bool isDead() const {
        return age() > lifetime;
    }

    [[nodiscard]] double age() const {
        return SECONDS() - born;
    }
};

class DestructionParticle : public DetonationParticle {
public:
    DestructionParticle(Texture &tex, std::mt19937& rng, int x=0, int y=0);
    ~DestructionParticle() override = default;

    void render(double dt) override;

private:
    SDL_Point vertpos[VERT_COUNT];
    double born;

    Color color { 0xF00000 };
    Texture &tex;
};

class EmberParticle : public DetonationParticle {
public:
    EmberParticle(std::mt19937& rng, int x=0, int y=0)
        : DetonationParticle(rng, x, y)
    {
        using random = std::uniform_real_distribution<>;
        using namespace Detonation::Particle;

        dx = random{Speed::MIN, Speed::MAX} (rng);
        dy = random(Speed::MIN, Speed::MAX) (rng);
        lifetime = random(Lifetime::MIN, Lifetime::MAX) (rng);
        size = random(Ember::Size::MIN, Ember::Size::MAX) (rng);

        color.g = std::uniform_real_distribution<>(Green::MIN, Green::MAX) (rng);
    }
    ~EmberParticle() override = default;

    void render(double dt) override {
        using namespace Detonation::Particle;
        x += dx * dt;
        y += dy * dt;
        color.a += Ember::DELTA_ALPHA * dt;

        color.draw();
        SDL_Rect fillrect;
        fillrect.w = size;
        fillrect.h = size;
        fillrect.x = x;
        fillrect.y = y;

        SDL_RenderFillRect(renderer, &fillrect);
    }

private:
    int size;
    Color color { 0xFF0000 };
};

class DetonationAnim : public Anim {
public:
    DetonationAnim(Texture &tex, std::mt19937& rng, SDL_Point pos, int size);
    ~DetonationAnim() override = default;

    void OnStart() override;
    bool OnUpdate(double  dt) override;

private:
    SDL_Point pos;
    std::vector<std::unique_ptr<DetonationParticle>> particles;

    double startTime;

    Texture &tex;
    std::mt19937& rng;

    void emitParticle(std::unique_ptr<DetonationParticle> part);
};


DetonationAnim::DetonationAnim(Texture &tex, std::mt19937& rng, SDL_Point tilePos, int size)
    : tex(tex), rng(rng)
{
    // Center of tile
    pos.x = tilePos.x + size / 2;
    pos.y = tilePos.y + size / 2;
}

void DetonationAnim::OnStart() {
    startTime = SECONDS();
}

void DetonationAnim::emitParticle(std::unique_ptr<DetonationParticle> part) {
    for (auto& ptr : particles) {
        if (ptr->isDead()) {
            ptr = std::move(part);
            return;
        }
    }

    // Nothing to recycle, add new particle
    particles.push_back(std::move(part));
}

bool DetonationAnim::OnUpdate(double dt) {
    using namespace Detonation;

    if (SECONDS() - startTime < Emitter::TIME) {
        if (particles.empty() || particles.back()->age() > Emitter::PERIOD) {
            for (int i = 0; i < Emitter::COUNT; ++i) {
                //particles.emplace_back(tex, rng, pos.x, pos.y);
                if (std::uniform_real_distribution<>(0.0, 1.0)(rng) < 0.5) {
                    emitParticle(std::make_unique<DestructionParticle>(tex, rng, pos.x, pos.y));
                }
                else {
                    emitParticle(std::make_unique<EmberParticle>(rng, pos.x, pos.y));
                }
            }
        }
    }

    bool rendering = false;
    // Iterate through particles for each depth
    for (int i = 0; i < Particle::Depth::MAX; ++i) {
        for (auto& particle : particles) {
            if (particle->depth == i && !particle->isDead()) {
                particle->render(dt);
                rendering = true;
            }
        }
    }

    return rendering;
}

DestructionParticle::DestructionParticle(Texture &tex, std::mt19937& rng, int x, int y)
    : DetonationParticle(rng, x, y), tex(tex)
{
    using randreal = std::uniform_real_distribution<>;
    using namespace Detonation::Particle;

    born = SECONDS();
    dx = randreal( Speed::MIN, Speed::MAX) (rng);
    dy = randreal( Speed::MIN, Speed::MAX) (rng);

    lifetime = randreal(Lifetime::MIN * 2, Lifetime::MAX * 2) (rng);
    color.g = randreal(Green::MIN, Green::MAX) (rng);

    x += randreal(-POS_VARIATION, POS_VARIATION)(rng);
    y += randreal(-POS_VARIATION, POS_VARIATION)(rng);

    constexpr int QUAD_SIDES = 4;
    SDL_Point quadverts[QUAD_SIDES];

    double theta = 0;
    for (int i = 0; i < QUAD_SIDES; ++i) {
        theta = randreal(theta, 2 * M_PI)(rng);
        //double r = randreal(15, 20)(rng);
        quadverts[i].x = int(cos(theta) * Piece::Size::MAX);
        quadverts[i].y = int(sin(theta) * Piece::Size::MAX);
    }

    // Convert quad to triangle vertices
    const int indices[VERT_COUNT] = {0, 1, 2, 2, 3, 0};
    for (int i = 0; i < VERT_COUNT; ++i) {
        vertpos[i] = quadverts[indices[i]];
    }

}


void DestructionParticle::render(double dt) {
    using namespace Detonation::Particle;
    x += dx * dt;
    y += dy * dt;
    color.a += Piece::DELTA_ALPHA * dt;

    if (age() > lifetime / 2) {
        dy = 0.0;
        dx = 0.0;
    }


    const SDL_FPoint quad[VERT_COUNT] = { {0, 0}, {1, 0}, {1, 1}, {1, 1}, {0, 1}, {0, 0} };
    const SDL_Color sdlcolor = color.as_sdl();
    SDL_Vertex vert[VERT_COUNT];

    for (int i = 0; i < VERT_COUNT; ++i) {
        vert[i].color = sdlcolor;
        vert[i].position.x = x + vertpos[i].x;
        vert[i].position.y = y + vertpos[i].y;
        vert[i].tex_coord = quad[i];
    }

    SDL_RenderGeometry(renderer, tex.raw(), vert, VERT_COUNT, nullptr, 0);
}


Mix_Chunk* Game::sounds[SoundEffects::COUNT];

constexpr int TITLE_SPACE_ABOVE = 15;
constexpr int TILE_SIZE = 32;
constexpr float PERCENT_MINES = 0.15;

constexpr int STARTING_SAFE_COUNT = 15;

namespace Difficulty {
    constexpr struct {int cols; int rows; } SIZES[] = { {10, 8}, {15, 12}, {20, 15} };
    const std::string STRINGS[] = {"Easy", "Medium", "Hard"};
    const Color COLORS[] = {
        { 0x008010 },
        { 0xF08000 },
        { 0xA03000 },
    };
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
    Tile::free();
    for (int i = 0; i < SoundEffects::COUNT; ++i) {
        Mix_FreeChunk(sounds[i]);
        sounds[i] = nullptr;
    }
}

Game::Game(SDL_Window *window) : Game(window, Difficulty::SIZES[1].rows, Difficulty::SIZES[1].cols) {}

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

    tileDatas.resize(rows*cols, TileSaveData::DEFAULT);
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
            Mix_PlayChannel(-1, sounds[currentHover->isFlagged() ? SoundEffects::FLAG : SoundEffects::WHOOSH], 0);
        }
    }
    else if (e.button == SDL_BUTTON_LEFT) {
        if (activeRestartButton().isMouseOver(e.x, e.y)) {
            return restartGame();
        }
        for (size_t i = 0; i < difficultyBtns.size(); ++i) {
            if (difficultyBtns[i].isMouseOver(e.x, e.y)) {
                rows = Difficulty::SIZES[i].rows;
                cols = Difficulty::SIZES[i].cols;
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
                    new DetonationAnim(Tile::backgrounds[TileBG::HIDDEN], rng, {mine.x, mine.y}, TILE_SIZE),
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
        Mix_PlayChannel(-1, sounds[SoundEffects::EXPLODE], 0);
    }
    else if (hasWon()) {
        printf("Game won!\n");
        onWon();
    }
    else {
        int channel = Mix_PlayChannel(-1, sounds[SoundEffects::BLIP], 0);
        if (channel != -1) {
            Mix_Volume(channel, 64);
        }
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

        const int NUMBTNS = sizeof(Difficulty::SIZES) / sizeof(Difficulty::SIZES[0]);
        difficultyBtns.resize(NUMBTNS, &mainFont);

        for (int i = 0; i < NUMBTNS; ++i) {
            auto& btn = difficultyBtns[i];
            btn.text.setColor(Difficulty::COLORS[i]);
            btn.text.setString(Difficulty::STRINGS[i]);
            btn.text.load();
            btn.setScale(0.4);
        }
    }

    for (int i = 0; i < SoundEffects::COUNT; ++i) {
        sounds[i] = Mix_LoadWAV(SOUND_FILES[i].c_str());
        if (sounds[i] == nullptr) {
            throw std::runtime_error("Failed to load sound:" + std::string(Mix_GetError()));
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



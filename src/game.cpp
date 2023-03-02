#include "game.h"
#include <SDL_mouse.h>
#include <string>
#include <algorithm>
#include "app.h"
#include "color.h"
#include <assert.h>
#include <cstring>
#include "frontend.h"

namespace Detonation {
    namespace Particle {
        namespace Speed {
            constexpr float MAX = 400.0;
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
            constexpr float DELTA_ALPHA = -0.4;

            namespace Size {
                constexpr float MIN = 6.0 / 32.0;
                constexpr float MAX = 10.0 / 32.0;
            }
        }

        namespace Piece {
            constexpr float CHANCE = 0.3;
            constexpr float DELTA_ALPHA = -0.1;

            namespace Size {
                constexpr float MAX = 15.0 / 32.0;
            }
        }
    }

    namespace Emitter {
        constexpr float PERIOD = 0.05;
        constexpr int COUNT = 10;
        constexpr float TIME = 3.0;
    }
}

constexpr int VERT_COUNT = 6;

constexpr Uint32 MINE_REVEAL_MILLISECONDS = 5000;
constexpr Uint32 FLIP_DELAY = 100;

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

float SOUND_VOLUMES[SoundEffects::COUNT] = {
    0.7,
    0.5,
    0.3,
    0.33,
};

std::string SOUND_FILES[SoundEffects::COUNT] = {
    "assets/sounds/flag.wav",
    "assets/sounds/whoosh.wav",
    "assets/sounds/blip.wav",
    "assets/sounds/explode.wav",
};

static void playSoundEffect(int effect) {
    int channel = Mix_PlayChannel(-1, Game::sounds[effect], 0);
    if (channel != -1) {
        Mix_Volume(channel, int(MIX_MAX_VOLUME * SOUND_VOLUMES[effect]));
    }
}

const char Save::HEADER[] = "MINE ";
const char *Save::FILE = "data.bin";

struct Quad { int l, r, t, b; };

class DetonationParticle {
protected:
    double lifetime;
    double dx;
    double dy;

    float x;
    float y;
    
    double born;

    SDL_Rect& field;

public:
    int depth;
    DetonationParticle(std::mt19937& rng, float x, float y, SDL_Rect &field)
        : x(x), y(y), field(field)
    {
        born = SECONDS();
        using namespace Detonation::Particle;
        depth = std::uniform_int_distribution<>(Depth::MIN, Depth::MAX) (rng);
    }
    virtual ~DetonationParticle() = default;

    virtual void render(double dt) = 0;
    void updatePosition(double dt, Quad quad) {
        float nextx = x + dx * dt;
        float nexty = y + dy * dt;
        if ((nextx + quad.l < field.x) || (nextx + quad.r > field.x + field.w)) {
            dx = -dx;
        } else {
            x = nextx;
        }

        if ((nexty + quad.t < field.y) || (nexty + quad.b > field.y + field.h)) {
            dy = -dy;
        } else {
            y = nexty;
        }
    }

    [[nodiscard]] bool isDead() const {
        return age() > lifetime;
    }

    [[nodiscard]] double age() const {
        return SECONDS() - born;
    }
};

class DestructionParticle : public DetonationParticle {
public:
    DestructionParticle(Texture &tex, std::mt19937& rng, int x, int y, SDL_Rect &field);
    ~DestructionParticle() override = default;

    void render(double dt) override;

private:
    SDL_Point vertpos[VERT_COUNT];
    double born;

    Color color { 0xF00000 };
    Texture &tex;
    Quad bounds{};
};

class EmberParticle : public DetonationParticle {
public:
    EmberParticle(std::mt19937& rng, int x, int y, SDL_Rect &field)
        : DetonationParticle(rng, x, y, field)
    {
        using random = std::uniform_real_distribution<>;
        using namespace Detonation::Particle;

        float speedMin = Speed::MIN;
        float speedMax = Speed::MAX;
        dx = random{speedMin, speedMax} (rng);
        dy = random{speedMin, speedMax} (rng);
        lifetime = random(Lifetime::MIN, Lifetime::MAX) (rng);
        size = random(Ember::Size::MIN * Tile::SIZE, Ember::Size::MAX * Tile::SIZE) (rng);

        color.g = std::uniform_real_distribution<>(Green::MIN, Green::MAX) (rng);
    }
    ~EmberParticle() override = default;

    void render(double dt) override {
        using namespace Detonation::Particle;

        Quad bounds = {0, size, 0, size};
        updatePosition(dt, bounds);

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
    DetonationAnim(Texture &tex, std::mt19937& rng, SDL_Point pos, SDL_Rect field);
    ~DetonationAnim() override = default;

    void OnStart() override;
    bool OnUpdate(double  dt) override;

private:
    SDL_Point pos;
    Texture &tex;
    std::mt19937& rng;
    SDL_Rect field;
    std::vector<std::unique_ptr<DetonationParticle>> particles;

    double startTime;

    void emitParticle(std::unique_ptr<DetonationParticle> part);
};


DetonationAnim::DetonationAnim(Texture &tex, std::mt19937& rng, SDL_Point tilePos, SDL_Rect field)
    : tex(tex), rng(rng), field(field)
{
    // Center of tile
    pos.x = tilePos.x + Tile::SIZE / 2;
    pos.y = tilePos.y + Tile::SIZE / 2;
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
                if (std::uniform_real_distribution<>(0.0, 1.0)(rng) < Particle::Piece::CHANCE) {
                    emitParticle(std::make_unique<DestructionParticle>(tex, rng, pos.x, pos.y, field));
                }
                else {
                    emitParticle(std::make_unique<EmberParticle>(rng, pos.x, pos.y, field));
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

DestructionParticle::DestructionParticle(Texture &tex, std::mt19937& rng, int x, int y, SDL_Rect &field)
    : DetonationParticle(rng, x, y, field), tex(tex)
{
    using randreal = std::uniform_real_distribution<>;
    using namespace Detonation::Particle;

    born = SECONDS();
    float speedMin = Speed::MIN;
    float speedMax = Speed::MAX;
    dx = randreal( speedMin, speedMax) (rng);
    dy = randreal( speedMin, speedMax) (rng);

    lifetime = randreal(Lifetime::MIN * 2, Lifetime::MAX * 2) (rng);
    color.g = randreal(Green::MIN, Green::MAX) (rng);

    x += randreal(-POS_VARIATION, POS_VARIATION)(rng);
    y += randreal(-POS_VARIATION, POS_VARIATION)(rng);

    constexpr int QUAD_SIDES = 4;
    SDL_Point quadverts[QUAD_SIDES];

    double theta = 0;
    float size = Piece::Size::MAX * Tile::SIZE;
    for (int i = 0; i < QUAD_SIDES; ++i) {
        theta = randreal(theta, 2*M_PI)(rng);
        auto& q = quadverts[i];
        q.x = int(cos(theta) * size);
        q.y = int(sin(theta) * size);
        if (q.x < bounds.l) bounds.l = q.x;
        if (q.x > bounds.r) bounds.r = q.x;
        if (q.y < bounds.t) bounds.t = q.y;
        if (q.y > bounds.b) bounds.b = q.y;
    }

    // Convert quad to triangle vertices
    const int indices[VERT_COUNT] = {0, 1, 2, 2, 3, 0};
    for (int i = 0; i < VERT_COUNT; ++i) {
        vertpos[i] = quadverts[indices[i]];
    }

}

void DestructionParticle::render(double dt) {
    using namespace Detonation::Particle;
    updatePosition(dt, bounds);

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

void Game::updateFlagCount() {
    int flagCount = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            flagCount += board[r][c].isFlagged();
        }
    }

    flagCounter.setString(std::to_string(flagCount)
                        + "/"
                        + std::to_string(mineCount)
                        + " flags");

    flagCounter.load();
    //flagCounter.x = (SCREEN_WIDTH - flagCounter.getWidth()) / 2;
}

void Game::OnUpdate(double dt) {
    int x, y; SDL_GetMouseState(&x, &y);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            auto &tile = board[r][c];
            tile.render(tile.isMouseOver(x, y));
            tile.animState.update(dt);
        }
    }

    animState.update(dt);

    for (auto btn : buttons) {
        if (!btn->hidden) {
            btn->render(btn->isMouseOver(x, y));
        }
    }

    flagCounter.render();
}

Game::~Game() {
    Tile::free();
    for (int i = 0; i < SoundEffects::COUNT; ++i) {
        Mix_FreeChunk(sounds[i]);
        sounds[i] = nullptr;
    }
}

Game::Game(SDL_Window *window)
    : rows(Difficulty::SIZES[1].rows)
    , cols(Difficulty::SIZES[1].cols)
    , rng(std::random_device{}())
    , mainFont("assets/fonts/Arbutus-Regular.ttf")
    , state(GameState::READY)
    , window(window)
    , flagCounter(mainFont.raw(), "0/? flags", 0xA00000)
    , restartBtn(mainFont.raw(), "Restart!", 0xFF1000)
    , playAgainBtn(mainFont.raw(), "Play again?", 0x00C000)
{
    loadMedia();
}

void Game::OnStart() {
    load();
    int loadedState = state;

    ready();

    if (!tileDatas.empty()) {
        for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
            board[r][c].load(tileDatas[r*cols+c]);
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

    resizeBoard();

    for (int row = 0; row < MAX_FIELD_SIZE; ++row) {
        for (int col = 0; col < MAX_FIELD_SIZE; ++col) {
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

void Game::save() {
    if (!openSaveWriter()) {
        puts("unable to write to save file");
        return;
    }

    for (size_t i = 0; i < sizeof(Save::HEADER)-1; ++i) {
        writeByte(Save::HEADER[i]);
    }

    writeByte('r');
    writeByte((Uint8)rows);
    writeByte('c');
    writeByte((Uint8)cols);

    writeByte('g');
    writeByte((Uint8)state);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            writeByte('t');
            writeByte(board[r][c].save());
        }
    }

    writeByte('\0');
    closeSaveFile();
}

void Game::load() {
    if (!openSaveReader()) {
        printf("no save file found\n");
        return;
    }
    for (const char *c = Save::HEADER; *c != '\0'; ++c) {
        Uint8 u8 = readByte();
        if (u8 != *c) {
            printf("Invalid or corrupted save file! Missing header.\n");
        }
    }
    Uint8 data;
    if ((data = readByte()) != 'r') {
        printf("Missing rows data :: expected r (%d) got (%d)\n", 'r', data);
        return;
    }
    rows = readByte();

    if (readByte() != 'c') {
        printf("Missing cols data\n");
        return;
    }
    cols = readByte();
    resizeBoard();

    if (readByte() != 'g') {
        printf("Missing game state\n");
        return;
    }
    state = readByte();

    tileDatas.resize(rows*cols, TileSaveData::DEFAULT);
    for (int i = 0; (data = readByte()) == 't'; i++) {
        tileDatas[i] = readByte();
    }

    closeSaveFile();
}

void Game::resizeBoard() {
    // Resize 2d vector

    assert(rows < MAX_FIELD_SIZE);
    assert(cols < MAX_FIELD_SIZE);
}

void Game::restartGame() {
    playAgainBtn.hidden = true;
    restartBtn.hidden = false;
    ready();
}

static Tile* getTileUnderMouse(Game& self, int mouseX, int mouseY) {
    for (int r = 0; r < self.rows; ++r) {
        for (int c = 0; c < self.cols; ++c) {
            if (self.board[r][c].isMouseOver(mouseX, mouseY)) {
                return &self.board[r][c];
            }
        }
    }
    return nullptr;
}

void Game::onClick(int x, int y) {
    Tile *currentHover = getTileUnderMouse(*this, x, y);
    if (!(state & GameState::OVER) && currentHover) {
        // Tile is hovered over
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
    } else {
        for (auto btn : buttons) {
            if (btn->onclick && btn->isMouseOver(x, y)) {
                btn->onclick();
                return;
            }
        }
    }
}

void Game::onAltClick(int x, int y) {
    if (state & GameState::OVER) return;

    Tile *currentHover = getTileUnderMouse(*this, x, y);

    if (currentHover && currentHover->isHidden()) {
        // Flag tile
        if (currentHover->isUnflagged()) {
            currentHover->flag();
        } else {
            currentHover->unflag();
        }
        playSoundEffect(currentHover->isFlagged() ? SoundEffects::FLAG : SoundEffects::WHOOSH);
    }
}

enum GameAnims {
    EXPLODE = 1,
};

void Game::onLost(Tile& mine) {
    state |= GameState::LOST;

    mine.animState.kill();
    mine.red();

    auto detonationAnim = new DetonationAnim {
        Tile::backgrounds[TileBG::HIDDEN],
        rng, {mine.x, mine.y},
        SDL_Rect{board[0][0].x, board[0][0].y, cols * Tile::SIZE, rows * Tile::SIZE },
    };
    animState.play(GameAnims::EXPLODE, detonationAnim);

    std::vector<Tile *> mines;
    double delay = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            auto &tile = board[r][c];
            if (tile.isMine() && tile.isUnflagged()) {
                mines.push_back(&tile);
            }
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

    const Uint32 deltaDelay = MINE_REVEAL_MILLISECONDS / mines.size();

    // Skip first item, the detonated mine
    for (auto it = mines.begin() + 1; it != mines.end(); ++it) {
        (*it)->flip(false, delay);
        delay += deltaDelay;
    }

    // When the last mine is revealed, mark all incorrect flags
    mines.back()->animState.anim->onstart = [this]() {
        for (int y = 0; y < MAX_FIELD_SIZE; y++) {
            for (int x = 0; x < MAX_FIELD_SIZE; x++) {
                auto &tile = board[y][x];
                if (tile.isSafe() && tile.isFlagged()) {
                    // Incorrect flag
                    tile.red();
                }
            }
        }
    };
}

void Game::onWon() {
    state |= GameState::WON;
    
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            auto &tile = board[r][c];
            if (tile.isMine()) {
                tile.dissapear();
            }
        }
    }
}

void Game::onRevealTile(Tile& revealed) {
    if (revealed.isMine()) {
        onLost(revealed);
        playSoundEffect(SoundEffects::EXPLODE);
    }
    else if (hasWon()) {
        onWon();
    }
    else {
        playSoundEffect(SoundEffects::BLIP);
    }

    if (state & GameState::OVER) {
        playAgainBtn.hidden = false;
        restartBtn.hidden = true;
    }
}

void Game::onMouseMove(SDL_MouseMotionEvent const& e) {
    (void)e;
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
    std::vector<Tile*> tiles;
    std::vector<Tile*> tospread;

    // Start by adding all neighbors, including diagonals
    pushHiddenNeighbors(root, tiles, true);

    while (count > 0 && !tiles.empty()) {
        // Select up to N random neighbors
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
        delay += FLIP_DELAY;
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
    // TODO: immediate-mode style UI
    Tile::loadMedia(mainFont.raw());

    restartBtn.setScale(0.5);
    restartBtn.load();

    playAgainBtn.setScale(0.5);
    playAgainBtn.load();
    playAgainBtn.hidden = true;

    restartBtn.onclick = [this](){ restartGame(); };
    playAgainBtn.onclick = [this](){ restartGame(); };

    {

        const int NUMBTNS = sizeof(Difficulty::SIZES) / sizeof(Difficulty::SIZES[0]);
        difficultyBtns.resize(NUMBTNS, mainFont.raw());

        for (int i = 0; i < NUMBTNS; ++i) {
            auto& btn = difficultyBtns[i];
            btn.text.setColor(Difficulty::COLORS[i]);
            btn.text.setString(Difficulty::STRINGS[i]);
        }
    }

    for (size_t i = 0; i < difficultyBtns.size(); ++i) {
        difficultyBtns[i].onclick = [this, i]() {
            rows = Difficulty::SIZES[i].rows;
            cols = Difficulty::SIZES[i].cols;
            resizeBoard();
            restartGame();
        };
    }

    for (int i = 0; i < SoundEffects::COUNT; ++i) {
        sounds[i] = Mix_LoadWAV((Sim.runtimeBasePath + SOUND_FILES[i]).c_str());
        if (sounds[i] == nullptr) {
            throw std::runtime_error("Failed to load sound:" + std::string(Mix_GetError()));
        }
    }

    buttons.push_back(&restartBtn);
    buttons.push_back(&playAgainBtn);
    for (auto& btn : difficultyBtns) buttons.push_back(&btn);

}

void Game::positionItems() {
    int y = 0;

    restartBtn.text.setColor(Color(0xFF1000));
    restartBtn.text.setString("Restart!");
    restartBtn.setScale(0.3);
    restartBtn.load();
    playAgainBtn.text.setColor(Color(0x00C000));
    playAgainBtn.text.setString("Play again?");
    playAgainBtn.setScale(0.3);
    playAgainBtn.load();

    y += playAgainBtn.getHeight() + 10;

    // Tiles
    //int x = (SCREEN_WIDTH - cols*Tile::SIZE) / 2;
    Tile::SIZE = std::min((SCREEN_WIDTH) / cols, (SCREEN_HEIGHT - y) / rows);
    int x = (SCREEN_WIDTH - cols*Tile::SIZE) / 2;
    Tile::reposition();
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            board[row][col].x = x + col * Tile::SIZE;
            board[row][col].y = y + row * Tile::SIZE;
        }
    }
    y += rows * 32;//Tile::SIZE;


    // Flag count text
    flagCounter.setScale(0.4);
    flagCounter.load();
    flagCounter.x = x;
    flagCounter.y = 10;


    x += cols * Tile::SIZE;
    y = 0;
    for (auto it = difficultyBtns.rbegin(); it != difficultyBtns.rend(); ++it) {
        it->setScale(0.3);
        it->load();
        x -= it->getWidth();
        it->setX(x);
        it->setY(y);
    }

    x -= std::max(playAgainBtn.getWidth(), restartBtn.getWidth());
    restartBtn.setX(x);
    restartBtn.setY(y);
    playAgainBtn.setX(x);
    playAgainBtn.setY(y);



    if (false && y > SCREEN_HEIGHT) {
        SDL_SetWindowSize(window, SCREEN_WIDTH, y);
    }
}

bool Game::hasWon() {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (board[r][c].isHidden() && board[r][c].isSafe()) {
                return false;
            }
        }
    }
    // All tiles that aren't mines have been revealed!
    return true;
}



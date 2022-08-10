#include "anim.h"

AnimState::AnimState() {
    active = false;
    anim = nullptr;
    //anim.reset();
    onstart = nullptr;
    onfinish = nullptr;
}

void AnimState::start(int code, Anim* anim_, callback onfinish, Uint32 delay) {
    active = code;
    started = false;
    startTime = SDL_GetTicks() + delay;
    //anim.reset(anim_);
    if (anim) delete anim;
    anim = anim_;
    this->onfinish = onfinish;
}

void AnimState::kill() {
    if (active) {
        active = 0;
        delete anim;
        anim = nullptr;
        if (onstart) onstart();
        // Remove onstart!
        onstart = nullptr;
        if (onfinish) onfinish();
    }
}

void AnimState::update(double dt) {
    if (anim == nullptr) return;

    if (!started) {
        if (SDL_GetTicks() >= startTime) {
            started = true;
            anim->OnStart();
            if (onstart) { onstart(); }
        }
    }
    else if (!anim->OnUpdate(dt)) {
        kill();
        return;
    }

}

// Base of flag poll in GIMP
const double FLAG_ROT_POINT_X = 18 / 64.0;
const double FLAG_ROT_POINT_Y = 57 / 64.0;

FlagAnim::FlagAnim(const Texture *flagTex, SDL_Point pos, bool&isFlagged) : flag(flagTex), pos(pos), isFlagged(isFlagged)
{
    rotPoint.x = (int)(FLAG_ROT_POINT_X * flag->getWidth());
    rotPoint.y = (int)(FLAG_ROT_POINT_Y * flag->getHeight());
}

void FlagAnim::OnStart() {
    angle = isFlagged ? 120.0 : 0.0;
}


bool FlagAnim::OnUpdate(double dt) {
    if (angle < 0.0 || angle > 120.0) {
        return false;
    }
    if (isFlagged) angle -= deltaAngle * dt;
    else angle += deltaAngle * dt;
    
    flag->render(pos.x, pos.y, nullptr, angle, &rotPoint);
    return true;
}


UncoverAnim::UncoverAnim(const Texture *hidden, SDL_Point pos, std::mt19937& rng)
    : hidden(hidden), pos(pos), rng(rng)
{
    deltaWidth = 0.0;
    deltaHeight = 0.0;
    widthPercent = 1.0;
    heightPercent = 1.0;
    inverseX = false;
    inverseY = false;
}

void UncoverAnim::OnStart() {

    double v = std::uniform_real_distribution<>(0.0, 1.0)(rng);
    if (v < 0.5) {
        deltaWidth = 1.0;
        inverseX = v < 0.25;
    }
    else {
        deltaHeight = 1.0;
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

    hidden->renderPart(pos.x, pos.y, &rect);

    return true;
}


DetonationAnim::DetonationAnim(std::mt19937& rng, SDL_Point tilePos, int size)
    : rng(rng)
{
    pos.x = tilePos.x + size / 2;
    pos.y = tilePos.y + size / 2;
}

void DetonationAnim::OnStart() {
    startTime = SDL_GetTicks() * 0.001;
}

bool DetonationAnim::OnUpdate(double dt) {
    if (SDL_GetTicks()*0.001 - startTime < PARTICLE_EMIT_TIME) {
        if (particles.empty() || particles.back().age() > 0.1) {
            for (int i = 0; i < 3; ++i)
            particles.emplace_back(rng, pos.x, pos.y);
        }
    }

    bool rendering = false;
    for (auto& particle : particles) {
        if (!particle.isDead()) {
            particle.render(dt);
            rendering = true;
        }
    }

    return rendering;
}

DetonationParticle::DetonationParticle(std::mt19937& rng, int x, int y)
    : x(x), y(y)
{
    const float MAX_SPEED = 150;
    born = SDL_GetTicks() * 0.001;
    dx = std::uniform_real_distribution<>{-MAX_SPEED, MAX_SPEED} (rng);
    dy = std::uniform_real_distribution<>(-MAX_SPEED, MAX_SPEED) (rng);
    lifetime = std::uniform_real_distribution<>(1.0, 2.0) (rng);

    color.g = std::uniform_real_distribution<>(0.0, 0.6) (rng);
}


const double DetonationParticle::DELTA_ALPHA = -0.2;


void DetonationParticle::render(double dt) {
    x += dx * dt;
    y += dy * dt;
    color.a += DELTA_ALPHA * dt;

    color.setToRender();
    SDL_Rect fillrect;
    fillrect.w = 12;
    fillrect.h = 12;
    fillrect.x = x;
    fillrect.y = y;

    SDL_RenderFillRect(renderer, &fillrect);
}

const double MineRevealAnim::DELTA_ALPHA = -1.0;

MineRevealAnim::MineRevealAnim(SDL_Point pos, int size) : pos(pos), size(size) {
    alpha = 1.0;
}

void MineRevealAnim::OnStart() {
}

bool MineRevealAnim::OnUpdate(double dt) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, (int)(alpha*0xFF));
    SDL_Rect fillrect { pos.x, pos.y, size, size };
    SDL_RenderFillRect(renderer, &fillrect);

    alpha += DELTA_ALPHA * dt;

    return alpha > 0.3;
}

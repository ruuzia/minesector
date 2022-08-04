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
        if (onfinish) onfinish();
    }
}

void AnimState::update(double dt) {
    if (anim == nullptr) return;

    if (!started) {
        if (SDL_GetTicks() >= startTime) {
            startTime = SDL_GetTicks();
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

FlagAnim::FlagAnim(Texture *flagTex, SDL_Point pos, bool&isFlagged) : flag(flagTex), pos(pos), isFlagged(isFlagged)
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
    
    flag->render(pos.x, pos.y, NULL, angle, &rotPoint);
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

    SDL_Rect rect = {
        .x = inverseX ? (int) (hidden->getWidth() * (1 - widthPercent)) : 0,
        .y = inverseY ? (int) (hidden->getHeight() * (1 - heightPercent)) : 0,
        .w = (int) (hidden->getWidth() * widthPercent),
        .h = (int) (hidden->getHeight() * heightPercent),
    };

    hidden->renderPart(pos.x, pos.y, &rect);

    return true;
}


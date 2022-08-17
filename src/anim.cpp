#include "anim.h"
#include <SDL.h>

namespace MineReveal {
    constexpr double DELTA_ALPHA = -1.0;
    constexpr double FINISHED_ALPHA = 0.3;
}

Anim::Anim() {
    onstart = nullptr;
    onfinish = nullptr;
}

AnimState::AnimState() {
    active = false;
    anim.reset();
    //anim.reset();
}

Anim& AnimState::play(int code, Anim* anim_, Uint32 delay) {
    // Destroy any already active anim
    kill();
    anim.reset(anim_);

    active = code;
    started = false;
    startTime = SDL_GetTicks() + delay;

    return *anim;
}

bool AnimState::isAnimActive(int code) {
    if (!anim) return false;
    return code == active;
}

void AnimState::kill() {
    if (anim) {
        // Currently, onstart and onfinish are guaranteed to be called,
        // even if the animation isn't able to start or finish
        if (not started && anim->onstart) anim->onstart();
        if (anim->onfinish) anim->onfinish();

        active = 0;
        anim.reset();
    }
}

void AnimState::update(double dt) {
    if (anim == nullptr) return;

    if (!started) {
        if (SDL_GetTicks() >= startTime) {
            started = true;
            anim->OnStart();
            if (anim->onstart) { anim->onstart(); }
        }
    }
    else if (!anim->OnUpdate(dt)) {
        kill();
        return;
    }

}

MineRevealAnim::MineRevealAnim(SDL_Point pos, int size) : pos(pos), size(size) {
    alpha = 1.0;
}

void MineRevealAnim::OnStart() {
}

bool MineRevealAnim::OnUpdate(double dt) {
    using namespace MineReveal;

    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, (int)(alpha*0xFF));
    SDL_Rect fillrect { pos.x, pos.y, size, size };
    SDL_RenderFillRect(renderer, &fillrect);

    alpha += DELTA_ALPHA * dt;

    return alpha > FINISHED_ALPHA;
}



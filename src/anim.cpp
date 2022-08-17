#include "anim.h"
#include <SDL.h>

namespace MineReveal {
    constexpr double DELTA_ALPHA = -1.0;
    constexpr double FINISHED_ALPHA = 0.3;
}


AnimState::AnimState() {
    active = false;
    anim.reset();
    //anim.reset();
    onstart = nullptr;
    onfinish = nullptr;
}

void AnimState::start(int code, Anim* anim_, callback onfinish, Uint32 delay) {
    // Destroy any already active anim
    kill();
    anim.reset(anim_);

    active = code;
    started = false;
    startTime = SDL_GetTicks() + delay;
    this->onfinish = onfinish;
}

bool AnimState::isAnimActive(int code) {
    if (!anim) return false;
    return code == active;
}

void AnimState::kill() {
    if (anim) {
        active = 0;
        anim.reset();

        // Currently, onstart and onfinish are guaranteed to be called,
        // even if the animation isn't able to start or finish
        if (not started && onstart) onstart();
        onstart = nullptr;
        if (onfinish) onfinish();
        onfinish = nullptr;
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



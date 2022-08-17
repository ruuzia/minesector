#ifndef ANIM_H
#define ANIM_H

#include "texture.h"
#include "color.h"
#include <functional>
#include <random>
#include <memory>

typedef std::function<void()> callback;

class Anim {
public:
    Anim();
    virtual ~Anim() = default;

    virtual void OnStart() = 0;
    virtual bool OnUpdate(double dt) = 0;

    callback onstart;
    callback onfinish;
};

class AnimState {
public:
    AnimState();

    void update(double dt);
    Anim& play(int code, Anim* anim, Uint32 delay = 0);
    void kill();
    Uint32 runningTime() {
        return SDL_GetTicks() - startTime;
    }

    bool isAnimActive(int code);

    Uint64 current;
    std::unique_ptr<Anim> anim;

private:
    int active;
    bool started;
    Uint32 startTime;
};

class MineRevealAnim : public Anim {
public:
    MineRevealAnim(SDL_Point pos, int size);
    ~MineRevealAnim() override = default;

    void OnStart() override;
    bool OnUpdate(double dt) override;

private:
    SDL_Point pos;
    int size;

    double alpha;
};



#endif

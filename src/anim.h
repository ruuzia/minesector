#ifndef ANIM_H
#define ANIM_H

#include "texture.h"
#include <functional>
#include <random>
#include <memory>

typedef std::function<void()> callback;

class Anim {
public:
    virtual ~Anim() = default;

    virtual void OnStart() = 0;
    virtual bool OnUpdate(double dt) = 0;
};

class AnimState {
public:
    AnimState();

    void update(double dt);
    void start(int code, Anim* anim, callback onfinish, Uint32 delay = 0);
    void kill();
    Uint32 runningTime() {
        return SDL_GetTicks() - startTime;
    }

    Uint64 current;
    int active;

    callback onstart;
private:
    bool started;
    Uint32 startTime;
    callback onfinish;

    Anim* anim;
    //std::unique_ptr<Anim> anim;
};


class FlagAnim : public Anim {
public:
    FlagAnim(Texture *flagTex, SDL_Point pos, bool& isFlagged);

    // Call virtual destructor
    ~FlagAnim() {}

    bool OnUpdate(double dt);
    void OnStart();

private:
    Texture* flag;
    SDL_Point pos;
    bool& isFlagged;
    double angle;
    SDL_Point rotPoint;
    float deltaAngle = 180.0;
};

class UncoverAnim : public Anim {
public:
    UncoverAnim(const Texture *hidden, SDL_Point pos, std::mt19937& rng);
    // Call virtual destructor, nothing to free
    ~UncoverAnim() {}

    void OnStart() override;
    bool OnUpdate(double dt) override;

private:
    const Texture *hidden;
    SDL_Point pos;
    std::mt19937 &rng;

    double widthPercent;
    double heightPercent;
    double deltaWidth;
    double deltaHeight;
    bool inverseX;
    bool inverseY;
};

#endif

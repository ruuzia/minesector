#ifndef ANIM_H
#define ANIM_H

#include "texture.h"
#include <functional>
#include <random>


typedef std::function<void()> callback;

class Anim {
public:
    virtual ~Anim() {}

    virtual void OnStart() = 0;
    virtual bool OnUpdate(double dt) = 0;
};

class AnimState {
public:
    AnimState();

    void update(double dt);
    void start(Anim* anim, callback onfinish);
    Uint32 runningTime() {
        return SDL_GetTicks() - startTime;
    }

    Uint64 current;
    bool active;

private:

    Uint32 startTime;
    Uint32 length;
    callback onfinish;

    Anim* anim;
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

    void OnStart();
    bool OnUpdate(double dt);

private:
    const Texture *hidden;
    SDL_Point pos;
    std::mt19937 &rng;

    SDL_Rect clip;
    double widthPercent;
    double heightPercent;
    double deltaWidth;
    double deltaHeight;
    bool inverseX;
    bool inverseY;
};

#endif

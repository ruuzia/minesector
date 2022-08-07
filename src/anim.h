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
    FlagAnim(const Texture *flagTex, SDL_Point pos, bool& isFlagged);

    // Call virtual destructor
    ~FlagAnim() {}

    bool OnUpdate(double dt);
    void OnStart();

private:
    const Texture* flag;
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

// If I add more particles, I'll make a parent Particle class
// But that's a lot of boilerplate for a single particle effect
class DetonationParticle {
public:
    DetonationParticle(std::mt19937& rng, const Texture*tex, int x=0, int y=0);
    ~DetonationParticle() = default;

    [[nodiscard]] bool isDead() const {
        return age() > lifetime;
    }

    void render(double dt);

    [[nodiscard]] double age() const {
        return (double)SDL_GetTicks() * 0.001 - born;
    }
private:
    double lifetime;
    double dx;
    double dy;

    float x;
    float y;
    std::mt19937& rng;
    double born;
    const Texture *texture;

    double colorAlpha;
    double colorRed;
    double colorGreen;
    double colorBlue;

    static const double DELTA_ALPHA;
};

class DetonationAnim : public Anim {
public:
    DetonationAnim(std::mt19937& rng, SDL_Point pos, const Texture *particle);
    ~DetonationAnim() override = default;

    void OnStart() override;
    bool OnUpdate(double  dt) override;

private:
    const Texture* particleTex;
    SDL_Point pos;
    std::vector<DetonationParticle> particles;


    double startTime;
    double PARTICLE_EMIT_TIME = 5.0;

    std::mt19937& rng;
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
    static const double DELTA_ALPHA;
};


#endif

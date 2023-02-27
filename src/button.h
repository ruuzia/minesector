#ifndef BUTTON_H
#define BUTTON_H

#include "texture.h"
#include <functional>
#include "text.h"


class Button {
public:
    Button(Texture *tex);
    Button() {
        background = nullptr;
    }
    virtual ~Button();

    const Texture* background;

    bool isMouseOver(int mouseX, int mouseY) const;

    virtual void mouseEnter() {}
    virtual void mouseLeave() {}

    std::function<void()> onclick;

    virtual void render(bool isSelected)  {
        (void)isSelected;
        background->render(x, y);
    }

    virtual int getWidth() const { return background->getWidth(); }
    virtual int getHeight() const { return background->getHeight(); }

    // Just leaving position public for now
    int x, y;

    bool hidden;
private:
};


class TextButton : public Button {
public:
    TextButton(TTF_Font * font = nullptr, std::string string = "", Color color = {0.f, 0.f, 0.f});
    ~TextButton();

    void render(bool isSelected) override;
    void load();

    int getWidth() const override;
    int getHeight() const override;

    void setY(int y_) {
        y = y_;
        text.y = y_ + borderWidth;
    }
    void setX(int x_) {
        x = x_;
        text.x = x_ + borderWidth;
    }

    void setCenterY(int y_) {
        setY(y_ - (text.getHeight() / 2) - borderWidth);
    }

    void setCenterX(int x_) {
        setX(x_ - (text.getWidth() / 2) - borderWidth);
    }

    void setCenterPos(int x_, int y_) {
        setCenterX(x_);
        setCenterY(y_);
    }

    void setPos(int x_, int y_) {
        setX(x_);
        setY(y_);
    }

    Text text;

    void setScale(double scale_) {
        scale = scale_;
        text.setScale(scale);
    }
    int borderWidth;
private:
    double scale;
    Color bgcolor;
    Color hoverbg;
};

#endif

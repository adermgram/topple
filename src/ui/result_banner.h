#pragma once

#include "raylib.h"
#include "ui/button.h"
#include "ui/theme.h"
#include <string>

namespace ui {

// End-of-round panel over a dimmed screen, with Retry and Menu buttons.
class ResultBanner {
public:
    enum class Action { None, Retry, Menu };

    ResultBanner();

    void Show(std::string title, std::string subtitle, Color accent);
    bool visible() const { return visible_; }

    Action Update(float dt);
    void Draw(const Theme& theme) const;

private:
    bool visible_ = false;
    float appear_ = 0.0f;   // 0..1 fade-in progress
    std::string title_;
    std::string subtitle_;
    Color accent_{255, 255, 255, 255};
    Button retryButton_;
    Button menuButton_;
};

}  // namespace ui

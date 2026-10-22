#pragma once

#include "raylib.h"
#include "ui/button.h"
#include "ui/theme.h"
#include <string>
#include <vector>

namespace ui {

// End-of-round panel over a dimmed screen, with a row of choice buttons.
class ResultBanner {
public:
    enum class Action { None, Retry, Next, Levels };

    struct Choice {
        Action action;
        std::string label;
        bool primary;
    };

    static constexpr int NO_STARS = -1;

    // stars (0..3) adds a row of stars that pop in one by one; NO_STARS leaves it out
    void Show(std::string title, std::string subtitle, Color accent,
              const std::vector<Choice>& choices, int stars = NO_STARS);
    bool visible() const { return visible_; }

    // Returns the chosen action on the frame a button is clicked
    Action Update(float dt);
    void Draw(const Theme& theme) const;

private:
    struct Entry {
        Action action;
        Button button;
    };

    bool visible_ = false;
    float appear_ = 0.0f;   // 0..1 fade-in progress
    float shown_ = 0.0f;    // seconds since Show, drives the star animation
    int stars_ = NO_STARS;
    std::string title_;
    std::string subtitle_;
    Color accent_{255, 255, 255, 255};
    std::vector<Entry> entries_;
};

}  // namespace ui

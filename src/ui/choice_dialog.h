#pragma once

#include "input/pointer.h"
#include "raylib.h"
#include "ui/button.h"
#include "ui/theme.h"
#include <string>
#include <vector>

namespace ui {

// A centred panel over a dimmed screen: a title, optional stars and a row of buttons.
// Used for the pause menu and for the result of a round.
class ChoiceDialog {
public:
    enum class Action { None, Resume, Retry, Next, Levels };

    struct Choice {
        Action action;
        std::string label;
        bool primary;
    };

    static constexpr int NO_STARS = -1;

    // stars (0..3) adds a row of stars that pop in one by one
    void Show(std::string title, std::string subtitle, Color accent,
              const std::vector<Choice>& choices, int stars = NO_STARS);
    void Hide();

    // The chosen action, on the frame its button is clicked
    Action Update(float dt, const Pointer& pointer);
    void Draw(const Theme& theme) const;

private:
    struct Entry {
        Action action;
        Button button;
    };

    bool visible_ = false;
    float appear_ = 0.0f;   // fade-in progress, 0..1
    float shown_ = 0.0f;    // seconds since Show
    int stars_ = NO_STARS;
    std::string title_;
    std::string subtitle_;
    Color accent_{255, 255, 255, 255};
    std::vector<Entry> entries_;
};

}  // namespace ui

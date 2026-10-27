#pragma once

#include "input/pointer.h"
#include "raylib.h"
#include "ui/button.h"
#include "ui/theme.h"
#include <string>
#include <vector>

namespace ui {

    class ChoiceDialog {
    public:
        enum class Action { None, Resume, Retry, Next, Levels };

        struct Choice {
            Action action;
            std::string label;
            bool primary;
        };

        static constexpr int NO_STARS = -1;

        void Show(std::string title, std::string subtitle, Color accent,
                const std::vector<Choice>& choices, int stars = NO_STARS);
        void Hide();

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

#pragma once

#include "raylib.h"
#include "ui/theme.h"
#include <string>

namespace ui {

// A rounded button with an eased hover highlight and a press dip.
class Button {
public:
    Button(std::string label, Rectangle bounds, bool primary = false);

    // Returns true on the frame the button is clicked
    bool Update(float dt);
    void Draw(const Theme& theme) const;

private:
    std::string label_;
    Rectangle bounds_;
    bool primary_;
    float hover_ = 0.0f;   // 0..1, eased toward the hover state
    bool pressed_ = false;
};

}  // namespace ui

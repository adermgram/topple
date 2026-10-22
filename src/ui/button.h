#pragma once

#include "input/pointer.h"
#include "raylib.h"
#include "ui/theme.h"
#include <string>

namespace ui {

// A rounded button with an eased hover highlight and a press dip
class Button {
public:
    Button(std::string label, Rectangle bounds, bool primary = false, float fontSize = 26.0f);

    // True on the frame the button is clicked (pressed and released over it)
    bool Update(float dt, const Pointer& pointer);
    void Draw(const Theme& theme) const;

private:
    std::string label_;
    Rectangle bounds_;
    bool primary_;
    float fontSize_;
    float hover_ = 0.0f;   // 0..1
    bool pressed_ = false;
};

}  // namespace ui

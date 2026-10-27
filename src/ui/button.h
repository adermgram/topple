#pragma once

#include "input/pointer.h"
#include "raylib.h"
#include "ui/theme.h"
#include <string>

namespace ui {

class Button {
public:
    Button(std::string label, Rectangle bounds, bool primary = false, float fontSize = 26.0f);

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

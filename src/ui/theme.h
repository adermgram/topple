#pragma once

#include "raylib.h"
#include <string>

namespace ui {

    struct Palette {
        Color bgTop{30, 36, 56, 255};
        Color bgBottom{14, 16, 26, 255};
        Color panel{38, 44, 66, 255};
        Color text{236, 238, 245, 255};
        Color textDim{150, 158, 182, 255};
        Color accent{255, 184, 76, 255};
        Color button{52, 60, 90, 255};
        Color buttonHover{74, 86, 128, 255};
        Color shadow{0, 0, 0, 90};
    };

    enum class FontWeight { Medium, SemiBold };
    enum class Align { Left, Center, Right };

    class Theme {
    public:
        Theme();
        ~Theme();
        Theme(const Theme&) = delete;
        Theme& operator=(const Theme&) = delete;

        const Palette& palette() const { return palette_; }

        void DrawText(const std::string& text, Vector2 pos, float size, Color color,
                    Align align = Align::Left, FontWeight weight = FontWeight::Medium) const;
        Vector2 Measure(const std::string& text, float size,
                        FontWeight weight = FontWeight::Medium) const;

        void DrawBackground() const;

    private:
        const Font& FontFor(FontWeight weight) const;

        Palette palette_;
        Font medium_;
        Font semiBold_;
    };

}  // namespace ui

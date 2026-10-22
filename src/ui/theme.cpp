#include "ui/theme.h"

namespace ui {

namespace {

constexpr int FONT_LOAD_SIZE = 64;   // rasterised once, then scaled; keeps text crisp

Font LoadThemeFont(const char* file, bool& loaded) {
    std::string path = std::string(GetApplicationDirectory()) + "resources/fonts/" + file;
    if (!FileExists(path.c_str())) return GetFontDefault();

    Font font = LoadFontEx(path.c_str(), FONT_LOAD_SIZE, nullptr, 0);
    if (font.texture.id == 0) return GetFontDefault();

    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    loaded = true;
    return font;
}

}  // namespace

Theme::Theme() {
    bool mediumLoaded = false, semiBoldLoaded = false;
    medium_ = LoadThemeFont("Poppins-Medium.ttf", mediumLoaded);
    semiBold_ = LoadThemeFont("Poppins-SemiBold.ttf", semiBoldLoaded);
    ownsFonts_ = mediumLoaded && semiBoldLoaded;
    if (mediumLoaded != semiBoldLoaded) {
        // Keep ownership simple: either both are ours or neither is
        if (mediumLoaded) UnloadFont(medium_);
        if (semiBoldLoaded) UnloadFont(semiBold_);
        medium_ = semiBold_ = GetFontDefault();
    }
}

Theme::~Theme() {
    if (ownsFonts_) {
        UnloadFont(medium_);
        UnloadFont(semiBold_);
    }
}

const Font& Theme::FontFor(FontWeight weight) const {
    return weight == FontWeight::SemiBold ? semiBold_ : medium_;
}

Vector2 Theme::Measure(const std::string& text, float size, FontWeight weight) const {
    return MeasureTextEx(FontFor(weight), text.c_str(), size, size * 0.02f);
}

void Theme::DrawText(const std::string& text, Vector2 pos, float size, Color color,
                     Align align, FontWeight weight) const {
    Vector2 extent = Measure(text, size, weight);
    if (align == Align::Center) pos.x -= extent.x / 2;
    else if (align == Align::Right) pos.x -= extent.x;
    DrawTextEx(FontFor(weight), text.c_str(), pos, size, size * 0.02f, color);
}

void Theme::DrawBackground() const {
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(),
                           palette_.bgTop, palette_.bgBottom);
}

}  // namespace ui

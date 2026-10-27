#include "ui/theme.h"

namespace ui {

    namespace {

        constexpr int FONT_LOAD_SIZE = 64;   
        constexpr float LETTER_SPACING = 0.02f;

        // Falls back to raylib's built-in font if the file is missing
        Font LoadThemeFont(const char* file) {
            const std::string path = std::string(GetApplicationDirectory()) + "resources/fonts/" + file;
            if (!FileExists(path.c_str())) return GetFontDefault();

            Font font = LoadFontEx(path.c_str(), FONT_LOAD_SIZE, nullptr, 0);
            if (font.texture.id == 0) return GetFontDefault();

            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            return font;
        }

        bool IsOwned(const Font& font) { return font.texture.id != GetFontDefault().texture.id; }

    }  // namespace

    Theme::Theme()
        : medium_(LoadThemeFont("Poppins-Medium.ttf")), semiBold_(LoadThemeFont("Poppins-SemiBold.ttf")) {}

    Theme::~Theme() {
        if (IsOwned(medium_)) UnloadFont(medium_);
        if (IsOwned(semiBold_)) UnloadFont(semiBold_);
    }

    const Font& Theme::FontFor(FontWeight weight) const {
        return weight == FontWeight::SemiBold ? semiBold_ : medium_;
    }

    Vector2 Theme::Measure(const std::string& text, float size, FontWeight weight) const {
        return MeasureTextEx(FontFor(weight), text.c_str(), size, size * LETTER_SPACING);
    }

    void Theme::DrawText(const std::string& text, Vector2 pos, float size, Color color,
                        Align align, FontWeight weight) const {
        const Vector2 extent = Measure(text, size, weight);
        if (align == Align::Center) pos.x -= extent.x / 2;
        else if (align == Align::Right) pos.x -= extent.x;
        DrawTextEx(FontFor(weight), text.c_str(), pos, size, size * LETTER_SPACING, color);
    }

    void Theme::DrawBackground() const {
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), palette_.bgTop, palette_.bgBottom);
    }

}  // namespace ui

#pragma once

namespace game {

constexpr int MAX_STARS = 3;

// Winning always earns a star; spare shots earn the rest.
inline int StarsForShotsLeft(int shotsLeft) {
    if (shotsLeft >= 2) return 3;
    if (shotsLeft == 1) return 2;
    return 1;
}

}  // namespace game

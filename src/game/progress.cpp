#include "game/progress.h"
#include "game/rating.h"
#include <algorithm>
#include <fstream>
#include <numeric>
#include <utility>

namespace game {

Progress::Progress(std::string path) : path_(std::move(path)) { Load(); }

int Progress::stars(std::size_t level) const {
    return level < stars_.size() ? stars_[level] : 0;
}

int Progress::totalStars() const {
    return std::accumulate(stars_.begin(), stars_.end(), 0);
}

void Progress::Record(std::size_t level, int stars) {
    stars = std::clamp(stars, 0, MAX_STARS);
    if (level >= stars_.size()) stars_.resize(level + 1, 0);
    if (stars <= stars_[level]) return;

    stars_[level] = stars;
    Save();
}

void Progress::Load() {
    std::ifstream file(path_);
    int value;
    while (file >> value) stars_.push_back(std::clamp(value, 0, MAX_STARS));
}

void Progress::Save() const {
    std::ofstream file(path_, std::ios::trunc);
    for (std::size_t i = 0; i < stars_.size(); i++) file << (i ? " " : "") << stars_[i];
    file << '\n';
}

}  // namespace game

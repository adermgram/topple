#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace game {

    /* The player's best star rating per level, saved to a small text file
    (one line of numbers, e.g. "3 2 0 0 0"). A missing or damaged file just means no progress. */
    class Progress {
    public:
        explicit Progress(std::string path);   

        int stars(std::size_t level) const;    // 0 for a level that was never won
        int totalStars() const;

 
        void Record(std::size_t level, int stars);

    private:
        void Load();
        void Save() const;

        std::string path_;
        std::vector<int> stars_;
    };

}  // namespace game

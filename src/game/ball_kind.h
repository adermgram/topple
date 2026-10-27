#pragma once

namespace game {

    enum class BallKind {
        Standard,   // the everyday ball
        Heavy,      // big and dense: knocks stone over
        Splitter,   // click mid-flight to split into three
        Fragment,   // one of the three pieces of a split Splitter (never in a level's ammo)
    };

}  // namespace game

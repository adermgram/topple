#include "game/level_def.h"
#include "game/target.h"
#include <utility>

namespace game {

namespace {

using layout::GROUND_TOP;

constexpr float PILLAR_W = 18.0f;
constexpr float PLANK_T = 16.0f;
constexpr float CRATE_SIZE = 36.0f;
constexpr float GAP = 4.0f;   // pieces start slightly apart and settle into place

// Stacks pieces on a surface. Each Add* takes the y of the surface to stand on and
// returns the y of the new top surface, so pieces can be piled on one another.
class Builder {
public:
    Builder(std::string name, int shots) {
        def_.name = std::move(name);
        def_.shots = shots;
    }

    // Two pillars with a roof plank across them
    float AddTower(float centerX, float baseY, float pillarHeight, float spread) {
        float pillarY = baseY - GAP - pillarHeight / 2;
        for (float side : {-1.0f, 1.0f})
            AddBlock(BlockKind::Plank, {centerX + side * spread / 2, pillarY}, {PILLAR_W, pillarHeight});

        float roofY = baseY - GAP - pillarHeight - GAP - PLANK_T / 2;
        AddBlock(BlockKind::Plank, {centerX, roofY}, {spread + PILLAR_W * 2, PLANK_T});
        return roofY - PLANK_T / 2;
    }

    float AddCrate(float centerX, float baseY, float size = CRATE_SIZE) {
        float y = baseY - GAP - size / 2;
        AddBlock(BlockKind::Crate, {centerX, y}, {size, size});
        return y - size / 2;
    }

    void AddTarget(float centerX, float baseY) {
        def_.targets.push_back({centerX, baseY - GAP - Target::RADIUS});
    }

    LevelDef Done() { return std::move(def_); }

private:
    void AddBlock(BlockKind kind, Vector2 center, Vector2 size) {
        def_.blocks.push_back({kind, center, size});
    }

    LevelDef def_;
};

LevelDef WarmUp() {
    Builder b("Warm Up", 4);
    b.AddTarget(900, GROUND_TOP);
    float crate = b.AddCrate(1040, GROUND_TOP, 44);
    b.AddTarget(1040, crate);
    return b.Done();
}

LevelDef TwinTowers() {
    Builder b("Twin Towers", 5);
    float roofA = b.AddTower(880, GROUND_TOP, 80, 80);
    b.AddTarget(880, GROUND_TOP);
    b.AddCrate(880, roofA);

    float roofB = b.AddTower(1060, GROUND_TOP, 110, 80);
    b.AddTarget(1060, GROUND_TOP);
    b.AddTarget(1060, roofB);
    return b.Done();
}

LevelDef HighGround() {
    Builder b("High Ground", 5);
    float top = GROUND_TOP;
    for (int i = 0; i < 3; i++) top = b.AddCrate(800, top);
    b.AddTarget(800, top);

    float roof = b.AddTower(1020, GROUND_TOP, 150, 80);
    b.AddTarget(1020, GROUND_TOP);
    b.AddTarget(1020, roof);
    return b.Done();
}

LevelDef Fortress() {
    Builder b("Fortress", 6);
    float roofLeft = b.AddTower(840, GROUND_TOP, 80, 80);
    float roofMid = b.AddTower(990, GROUND_TOP, 110, 80);
    float roofRight = b.AddTower(1140, GROUND_TOP, 80, 80);

    for (float x : {840.0f, 990.0f, 1140.0f}) b.AddTarget(x, GROUND_TOP);
    b.AddTarget(990, roofMid);
    b.AddCrate(840, roofLeft);
    b.AddCrate(1140, roofRight);
    return b.Done();
}

LevelDef TwoStories() {
    Builder b("Two Stories", 6);
    float floor1 = b.AddTower(1000, GROUND_TOP, 90, 120);
    b.AddTarget(1000, GROUND_TOP);

    float floor2 = b.AddTower(1000, floor1, 80, 100);
    b.AddTarget(1000, floor1);
    b.AddTarget(1000, floor2);

    float shield = b.AddCrate(880, GROUND_TOP);
    b.AddCrate(880, shield);

    float crate = b.AddCrate(1140, GROUND_TOP);
    b.AddTarget(1140, crate);
    return b.Done();
}

}  // namespace

const std::vector<LevelDef>& Levels() {
    static const std::vector<LevelDef> levels = {WarmUp(), TwinTowers(), HighGround(), Fortress(), TwoStories()};
    return levels;
}

}  // namespace game

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

// Short names for writing ammo lists
constexpr BallKind S = BallKind::Standard;
constexpr BallKind H = BallKind::Heavy;
constexpr BallKind X = BallKind::Splitter;

// Stacks pieces on a surface. Each Add* takes the y of the surface to stand on and
// returns the y of the new top surface, so pieces can be piled on one another.
class Builder {
public:
    Builder(std::string name, std::vector<BallKind> ammo) {
        def_.name = std::move(name);
        def_.ammo = std::move(ammo);
    }

    // A box of any material standing on baseY
    float AddBox(BlockKind kind, float centerX, float baseY, float width, float height) {
        float y = baseY - GAP - height / 2;
        def_.blocks.push_back({kind, {centerX, y}, {width, height}});
        return y - height / 2;
    }

    // Two pillars with a roof plank across them
    float AddTower(float centerX, float baseY, float pillarHeight, float spread,
                   BlockKind pillars = BlockKind::Plank, BlockKind roof = BlockKind::Plank) {
        float pillarTop = baseY;
        for (float side : {-1.0f, 1.0f})
            pillarTop = AddBox(pillars, centerX + side * spread / 2, baseY, PILLAR_W, pillarHeight);
        return AddBox(roof, centerX, pillarTop, spread + PILLAR_W * 2, PLANK_T);
    }

    float AddCrate(float centerX, float baseY, float size = CRATE_SIZE) {
        return AddBox(BlockKind::Crate, centerX, baseY, size, size);
    }

    void AddTarget(float centerX, float baseY) {
        def_.targets.push_back({centerX, baseY - GAP - Target::RADIUS});
    }

    LevelDef Done() { return std::move(def_); }

private:
    LevelDef def_;
};

LevelDef WarmUp() {
    Builder b("Warm Up", {S, S, S, S});
    b.AddTarget(900, GROUND_TOP);
    float crate = b.AddCrate(1040, GROUND_TOP, 44);
    b.AddTarget(1040, crate);
    return b.Done();
}

LevelDef TwinTowers() {
    Builder b("Twin Towers", {S, S, S, S, S});
    float roofA = b.AddTower(880, GROUND_TOP, 80, 80);
    b.AddTarget(880, GROUND_TOP);
    b.AddCrate(880, roofA);

    float roofB = b.AddTower(1060, GROUND_TOP, 110, 80);
    b.AddTarget(1060, GROUND_TOP);
    b.AddTarget(1060, roofB);
    return b.Done();
}

// Two separate structures: a splitter can reach both
LevelDef HighGround() {
    Builder b("High Ground", {S, S, X, S, S});
    float top = GROUND_TOP;
    for (int i = 0; i < 3; i++) top = b.AddCrate(800, top);
    b.AddTarget(800, top);

    float roof = b.AddTower(1020, GROUND_TOP, 150, 80);
    b.AddTarget(1020, GROUND_TOP);
    b.AddTarget(1020, roof);
    return b.Done();
}

LevelDef Fortress() {
    Builder b("Fortress", {S, H, S, S, H, S});
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
    Builder b("Two Stories", {S, S, S, S, S, S});
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

// Glass breaks on a hard hit, so a crate resting on a glass roof drops onto the target below
LevelDef GlassHouse() {
    Builder b("Glass House", {S, S, S, S, S});
    float roofA = b.AddTower(870, GROUND_TOP, 90, 90, BlockKind::Glass, BlockKind::Glass);
    b.AddTarget(870, GROUND_TOP);
    b.AddCrate(870, roofA);

    float roofB = b.AddTower(1050, GROUND_TOP, 120, 90, BlockKind::Glass, BlockKind::Plank);
    b.AddTarget(1050, GROUND_TOP);
    b.AddTarget(1050, roofB);
    return b.Done();
}

// A stone wall can't be broken or pushed: lob the ball over it
LevelDef StoneWall() {
    Builder b("Stone Wall", {S, H, S, H, S});
    float wall = GROUND_TOP;
    for (int i = 0; i < 2; i++) wall = b.AddBox(BlockKind::Stone, 860, wall, 44, 70);

    float roof = b.AddTower(1030, GROUND_TOP, 90, 80, BlockKind::Plank, BlockKind::Glass);
    b.AddTarget(1030, GROUND_TOP);
    b.AddCrate(1030, roof);

    float crate = b.AddCrate(1170, GROUND_TOP, 40);
    b.AddTarget(1170, crate);
    return b.Done();
}

// Heavy stone slabs rest on glass pillars: break the glass and the stone comes down
LevelDef Crusher() {
    Builder b("Crusher", {S, S, X, S, S});
    b.AddTower(900, GROUND_TOP, 100, 90, BlockKind::Glass, BlockKind::Stone);
    b.AddTarget(900, GROUND_TOP);

    float roofB = b.AddTower(1100, GROUND_TOP, 130, 90, BlockKind::Glass, BlockKind::Stone);
    b.AddTarget(1100, GROUND_TOP);
    b.AddTarget(1100, roofB);
    return b.Done();
}

// Stone columns and a stone lid: only the heavy ball has the weight to topple them
LevelDef IronFortress() {
    Builder b("Iron Fortress", {H, H, S, H, S});
    float top = GROUND_TOP;
    b.AddBox(BlockKind::Stone, 880, GROUND_TOP, 36, 120);
    top = b.AddBox(BlockKind::Stone, 980, GROUND_TOP, 36, 120);
    float lid = b.AddBox(BlockKind::Stone, 930, top, 140, 16);
    b.AddTarget(930, GROUND_TOP);
    b.AddTarget(930, lid);

    float crates = GROUND_TOP;
    for (int i = 0; i < 3; i++) crates = b.AddCrate(1120, crates);
    b.AddTarget(1120, crates);
    return b.Done();
}

// Targets spread along the ground, each behind its own cover: one splitter can fan out over all of them
LevelDef Spread() {
    Builder b("Spread", {X, X, X, S, S});
    b.AddTarget(800, GROUND_TOP);

    float crate = b.AddCrate(900, GROUND_TOP);
    b.AddTarget(900, crate);

    b.AddTower(1030, GROUND_TOP, 90, 70);
    b.AddTarget(1030, GROUND_TOP);

    float crateB = b.AddCrate(1160, GROUND_TOP, 40);
    b.AddTarget(1160, crateB);
    return b.Done();
}

}  // namespace

const std::vector<LevelDef>& Levels() {
    static const std::vector<LevelDef> levels = {WarmUp(),      TwinTowers(), HighGround(), Fortress(),    TwoStories(),
                                                 GlassHouse(),  StoneWall(),  Crusher(),    IronFortress(), Spread()};
    return levels;
}

}  // namespace game

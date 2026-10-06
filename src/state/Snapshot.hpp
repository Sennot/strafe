#pragma once
/**
 * ghost::state — checkpoint snapshots.
 *
 * DIVISION OF LABOUR:
 *   The vanilla CheckpointObject (what GD itself stores per practice
 *   checkpoint) already restores: both players' physics (pos/vel/rotation/
 *   gravity/gamemode/size), and a decent chunk of gameplay state (GD 2.2
 *   improved practice restore: counters, some trigger states).
 *
 *   Our ExtendedSnapshot exists for everything vanilla does NOT restore —
 *   the "absolutely exact freeze" the user asked for (particles, movers, saw
 *   positions, trigger group timelines...). That fidelity work is PHASED,
 *   see ARCHITECTURE.md "Fidelity ladder". Do not add a field here without
 *   implementing its capture AND restore, or restore silently lies.
 */
#include <Geode/Geode.hpp>

namespace ghost {
using namespace geode::prelude;

struct ExtendedSnapshot {
    float percent = 0.f;     ///< run percent at capture
    double wallClock = 0.;   ///< session wall time at capture (Time-label semantics)

    // ---- P4 candidate fields (add WITH capture+restore, never before) ----
    // std::map<std::string, double> counters;         ///< m_gameState counters
    // std::set<short> activeGroups;                   ///< toggled/active groups
    // struct { float x, y, zoom; } camera;
    // double musicPosition;                           ///< fmod channel position
    // std::map<int, ParticleState> particles;         ///< per-system particle arrays
};

struct BankedCheckpoint {
    Ref<CheckpointObject> vanilla;   ///< engine's own checkpoint (P3: wire the
                                     ///< Ref to the object created this frame)
    ExtendedSnapshot extended;
};

} // namespace ghost

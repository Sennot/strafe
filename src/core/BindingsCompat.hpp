#pragma once
/**
 * ghost::compat — the ONE place where uncertain GD 2.2081 / Geode 5.7.1
 * binding details are isolated. Every mark says what to verify and how.
 *
 * RULE FOR CONTRIBUTORS (human or AI):
 *   If a game field/function name is not 100% certain, it goes here behind a
 *   helper with a VERIFY(...) comment, never sprinkled through feature code.
 *   After verifying at runtime (see docs/ROADMAP.md phase 0), remove the mark.
 */
#include <Geode/Geode.hpp>
#include <functional>
#include <string>

namespace ghost::compat {
using namespace geode::prelude;

/**
 * Current run percent (0..100) of the local player.
 *
 * VERIFY(runtime): GJBaseGameLayer::m_gameState.m_currentProgress is the
 * engine-truth percent used by GD 2.2 percent mods. If the field is not
 * present in Geode 5.7.1 bindings, fall back to the position math below
 * (accurate enough for banking semantics, may differ by ~0.1% on odd levels).
 */
inline float currentPercent(PlayLayer* pl) {
    if (pl == nullptr) return 0.f;

    // Primary: engine-truth progress. (VERIFY: keep if it compiles on 5.7.1)
    auto progress = pl->m_gameState.m_currentProgress;
    if (progress > 0.0001f) return progress;

    // Fallback: player position over level length.
    auto* p1 = pl->m_player1;
    if (p1 == nullptr || pl->m_level == nullptr) return 0.f;
    float length = pl->m_level->m_levelLength; // VERIFY: float field on GJGameLevel
    if (length <= 0.f) return 0.f;
    float pct = p1->getPositionX() / length * 100.f;
    if (pct < 0.f) pct = 0.f;
    if (pct > 100.f) pct = 100.f;
    return pct;
}

/**
 * Remove every practice checkpoint so the next respawn is from 0%.
 *
 * VERIFY(runtime): PlayLayer::removeAllCheckpoints() should exist on 2.2;
 * if not, iterate pl->m_checkpointArray and removeAllObjects() instead.
 */
inline void removeAllCheckpointsCompat(PlayLayer* pl) {
    if (pl == nullptr) return;
    pl->removeAllCheckpoints(); // VERIFY
}

/** True while the engine is in practice mode (field on PlayLayer). */
inline bool isPractice(PlayLayer* pl) {
    return pl != nullptr && pl->m_isPracticeMode;
}

/** Recursive node walk. */
void walkTree(CCNode* root, const std::function<void(CCNode*)>& fn);

/** First label in the subtree whose text starts with `prefix` (CI), or null. */
CCLabelProtocol* findLabelByPrefix(CCNode* root, const std::string& prefix);

} // namespace ghost::compat

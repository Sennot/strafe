#pragma once
/**
 * ghost::PracticeUICleaner — removes every visible trace of practice mode.
 *
 * What gets cleaned:
 *   - the bottom-left place/delete checkpoint buttons (UILayer);
 *   - the checkpoint diamonds that appear in the level when a checkpoint is
 *     placed (node-diff around PlayLayer::createCheckpoint — version-proof,
 *     no hardcoded tags);
 *   - "Practice Mode" texts (pause menu title).
 *
 * Everything hidden is remembered and restored when the session ends, so
 * leaving practice mode returns the UI to its true state.
 *
 * REFINEMENT WORKFLOW (important for the next contributor):
 *   Turn on the "Debug: dump node trees" setting, enter practice, read the
 *   Geode log, then tighten the filters below. The dump prints every node's
 *   class, Geode ID and label text. See docs/ROADMAP.md phase 1.
 */
#include <Geode/Geode.hpp>
#include <unordered_set>
#include <vector>

namespace ghost {
using namespace geode::prelude;

class PracticeUICleaner {
public:
    static PracticeUICleaner& get();

    /// Called when a disguised session starts (practice toggled ON).
    void onSessionStart(UILayer* uiLayer);

    /// Hide the checkpoint place/delete buttons. Heuristic in P0 (bottom-left
    /// menus); refine after a node dump. Idempotent.
    void onUILayer(UILayer* uiLayer);

    /// Take a node census right before PlayLayer::createCheckpoint runs.
    void beforeCreateCheckpoint(PlayLayer* pl);

    /// After createCheckpoint: hide every visual node that appeared since the
    /// census (the checkpoint diamond and any auto-checkpoint UI).
    void afterCreateCheckpoint(PlayLayer* pl);

    /// Pause menu: retitle "Practice Mode" -> "Paused".
    void fixPauseLayer(CCNode* pauseLayer);

    /// Restore everything hidden this session (practice OFF / quit).
    void restoreAll();

    /// Debug helper: log the whole subtree (class, id, tag, label text).
    void dumpTree(CCNode* root, const char* tag) const;

private:
    void hide(CCNode* node, const char* why);

    std::unordered_set<CCNode*> m_census;   ///< nodes alive before checkpoint
    std::vector<CCNode*> m_hidden;          ///< everything we hid (to restore)
};

} // namespace ghost

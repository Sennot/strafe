#pragma once
/**
 * ghost::LabelFreezer — freezes overlay label texts at checkpoint time.
 *
 * Confirmed facts about Mega Hack v9 labels (docs/FEASIBILITY.md):
 *   - they are cocos2d labels (RTTI: CCLabelBMFont / CCLabelProtocol, plus
 *     Geode 5.9+ geode::Label which is a CCLabelBMFont subclass) living in
 *     the game's scene tree, so they are reachable WITHOUT touching MH's code;
 *   - texts look like "FPS 240", "CPS 2|4|54", "Best Run: 37%",
 *     "Attempt 1";
 *   - some carry Geode node IDs ("checkpoints-label", "percentage-label").
 *
 * Strategy (layered, most robust wins):
 *   B) PIN: at freeze time snapshot the text of every matching label; every
 *      frame re-setString the snapshot if the text drifted. Defeats any
 *      updater because we always write AFTER them (we run in PlayLayer::update
 *      post-call, their label refresh happens during the same frame earlier
 *      or via their own scheduler).
 *   C) MATCH CONFIG: text prefixes + node IDs come from mod settings, so new
 *      label types are a config change, not a code change.
 *
 * (Layer A — calling MH's exported API — lives in mh/MegahackInterop and is
 *  used for richer integration, not required for freezing.)
 */
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

namespace ghost {
using namespace geode::prelude;

class LabelFreezer {
public:
    static LabelFreezer& get();

    /// (Re)read prefix/ID lists from mod settings.
    void loadConfig();

    /// Snapshot texts of all matching labels under `subtreeRoot`.
    void freeze(CCNode* subtreeRoot);

    /// Stop pinning; labels resume updating.
    void unfreeze();

    /// Call once per frame (PlayLayer::update post-hook). Re-pins drifted
    /// texts and recovers frozen labels that MH re-created (node identity
    /// changed but the text pattern still matches).
    void onFrame(CCNode* subtreeRoot);

    bool frozen() const { return m_frozen; }
    size_t frozenCount() const { return m_frozen.size(); }

    /// Log every label in the subtree (id + text) — the tool used to discover
    /// new patterns at runtime. Toggle: debug-dump-tree setting.
    void debugDump(CCNode* subtreeRoot) const;

private:
    struct Frozen {
        CCNode* node = nullptr;   ///< may go stale (MH can recreate labels)
        std::string text;         ///< the frozen text
        std::string nodeId;       ///< "" if it had no Geode ID
    };

    /// true if node matches config (prefix or node ID); fills current text.
    bool matches(CCNode* node, std::string& outText) const;

    std::vector<std::string> m_prefixes;
    std::vector<std::string> m_nodeIds;
    std::vector<Frozen> m_frozen;
    bool m_frozen = false;
};

} // namespace ghost

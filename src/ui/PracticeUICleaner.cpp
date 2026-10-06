#include "ui/PracticeUICleaner.hpp"
#include "core/BindingsCompat.hpp"
#include "core/ModConfig.hpp"

namespace ghost {

PracticeUICleaner& PracticeUICleaner::get() {
    static PracticeUICleaner instance;
    return instance;
}

void PracticeUICleaner::hide(CCNode* node, const char* why) {
    if (node == nullptr || !node->isVisible()) return;
    node->setVisible(false);
    m_hidden.push_back(node);
    log::debug("[GhostPractice] hid node '{}' ({})", node->getID(), why);
}

void PracticeUICleaner::onSessionStart(UILayer* uiLayer) {
    onUILayer(uiLayer);
}

void PracticeUICleaner::onUILayer(UILayer* uiLayer) {
    if (uiLayer == nullptr) return;

    const auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Heuristic (P0): practice checkpoint buttons are the menu(s) anchored at
    // the bottom-left corner. VERIFY(refine after node dump): replace the
    // position heuristic with exact Geode node IDs / tags once dumped.
    compat::walkTree(uiLayer, [&](CCNode* node) {
        if (dynamic_cast<CCMenu*>(node) == nullptr) return;
        auto pos = node->getPosition();
        if (pos.x < winSize.width * 0.35f && pos.y < winSize.height * 0.25f) {
            hide(node, "bottom-left menu (practice buttons heuristic)");
        }
    });
}

void PracticeUICleaner::beforeCreateCheckpoint(PlayLayer* pl) {
    m_census.clear();
    if (pl == nullptr) return;
    compat::walkTree(pl, [&](CCNode* node) { m_census.insert(node); });
}

void PracticeUICleaner::afterCreateCheckpoint(PlayLayer* pl) {
    if (pl == nullptr) return;
    compat::walkTree(pl, [&](CCNode* node) {
        if (m_census.count(node) != 0) return;       // existed before
        if (node == pl) return;
        // Only hide visual nodes; data-only objects are not in the tree anyway.
        hide(node, "appeared during createCheckpoint (checkpoint diamond)");
    });
    m_census.clear();
}

void PracticeUICleaner::fixPauseLayer(CCNode* pauseLayer) {
    if (pauseLayer == nullptr) return;
    if (auto* title = compat::findLabelByPrefix(pauseLayer, "Practice")) {
        title->setString("Paused");
        log::debug("[GhostPractice] retitled pause menu");
    }
}

void PracticeUICleaner::restoreAll() {
    for (auto* node : m_hidden) {
        if (node != nullptr) node->setVisible(true);
    }
    m_hidden.clear();
    m_census.clear();
}

void PracticeUICleaner::dumpTree(CCNode* root, const char* tag) const {
    log::info("[GhostPractice] ===== {} tree dump =====", tag);
    int depth = 0;
    // simple recursive printer with class + id + tag + text
    std::function<void(CCNode*, int)> rec = [&](CCNode* node, int d) {
        std::string pad(d * 2, ' ');
        std::string extra;
        if (auto* lp = dynamic_cast<CCLabelProtocol*>(node)) {
            const char* s = lp->getString();
            if (s) extra += std::string(" text='") + s + "'";
        }
        log::info(
            "[GhostPractice] {}{} id='{}' tag={} pos=({:.0f},{:.0f}){}",
            pad, typeid(*node).name(), node->getID(), node->getTag(),
            node->getPositionX(), node->getPositionY(), extra
        );
        if (auto* children = node->getChildren()) {
            for (auto* child : CCArrayExt<CCNode*>(children)) rec(child, d + 1);
        }
    };
    if (root != nullptr) rec(root, 0);
    log::info("[GhostPractice] ===== end {} dump =====", tag);
    (void)depth;
}

} // namespace ghost

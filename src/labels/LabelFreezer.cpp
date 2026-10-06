#include "labels/LabelFreezer.hpp"
#include "core/BindingsCompat.hpp"
#include "core/ModConfig.hpp"

namespace ghost {

LabelFreezer& LabelFreezer::get() {
    static LabelFreezer instance;
    return instance;
}

void LabelFreezer::loadConfig() {
    m_prefixes = cfgCsv("freeze-label-patterns");
    m_nodeIds = cfgCsv("freeze-label-node-ids");
}

bool LabelFreezer::matches(CCNode* node, std::string& outText) const {
    // RTTI sidecast to the label interface covers CCLabelBMFont, CCLabelTTF,
    // CCLabelAtlas and Geode's geode::Label alike.
    auto* lp = dynamic_cast<CCLabelProtocol*>(node);
    if (lp == nullptr) return false;

    const char* s = lp->getString();
    if (s == nullptr) return false;
    outText = s;

    for (const auto& id : m_nodeIds) {
        if (node->getID() == id) return true;  // Geode node-ID extension
    }
    for (const auto& p : m_prefixes) {
        if (startsWithCI(outText, p)) return true;
    }
    return false;
}

void LabelFreezer::freeze(CCNode* subtreeRoot) {
    m_frozen.clear();
    loadConfig();

    compat::walkTree(subtreeRoot, [&](CCNode* node) {
        std::string text;
        if (matches(node, text)) {
            m_frozen.push_back(Frozen{ node, text, node->getID() });
        }
    });

    m_frozen = true;
    log::info("[GhostPractice] labels FROZEN ({} matched)", m_frozen.size());
}

void LabelFreezer::unfreeze() {
    if (!m_frozen) return;
    m_frozen.clear();
    m_frozen = false;
    log::info("[GhostPractice] labels unfrozen");
}

void LabelFreezer::onFrame(CCNode* subtreeRoot) {
    if (!m_frozen) return;

    // 1) Re-pin drifted texts (only write when text changed — setString is
    //    not free for BMFont labels).
    int stale = 0;
    for (auto& f : m_frozen) {
        if (f.node == nullptr) { ++stale; continue; }
        auto* lp = dynamic_cast<CCLabelProtocol*>(f.node);
        if (lp == nullptr) { ++stale; continue; }
        const char* s = lp->getString();
        if (s == nullptr || f.text != s) {
            lp->setString(f.text.c_str());
        }
    }

    // 2) If nodes went stale (MH recreated its labels), re-acquire by
    //    pattern/ID and keep the ORIGINAL frozen text.
    if (stale > 0) {
        std::vector<Frozen> refreshed;
        refreshed.reserve(m_frozen.size());
        compat::walkTree(subtreeRoot, [&](CCNode* node) {
            std::string text;
            if (!matches(node, text)) return;
            // keep the previously frozen text if this pattern was frozen
            for (const auto& f : m_frozen) {
                if (f.node == node) return;  // still alive, already pinned
            }
            // find any stale entry with the same node id or same prefix family
            for (const auto& f : m_frozen) {
                if (f.node == nullptr) continue;
                bool sameId = !f.nodeId.empty() && f.nodeId == node->getID();
                bool sameText = false;
                if (!sameId) {
                    for (const auto& p : m_prefixes) {
                        if (startsWithCI(f.text, p) && startsWithCI(text, p)) {
                            sameText = true;
                            break;
                        }
                    }
                }
                if (sameId || sameText) {
                    refreshed.push_back(Frozen{ node, f.text, node->getID() });
                    return;
                }
            }
        });
        if (!refreshed.empty()) {
            for (auto& r : refreshed) m_frozen.push_back(std::move(r));
            // drop entries whose node is gone
            std::erase_if(m_frozen, [](const Frozen& f) {
                return f.node != nullptr &&
                       dynamic_cast<CCLabelProtocol*>(f.node) == nullptr;
            });
        }
    }
}

void LabelFreezer::debugDump(CCNode* subtreeRoot) const {
    log::info("[GhostPractice] --- label dump begin ---");
    compat::walkTree(subtreeRoot, [](CCNode* node) {
        auto* lp = dynamic_cast<CCLabelProtocol*>(node);
        if (lp == nullptr) return;
        const char* s = lp->getString();
        log::info(
            "[GhostPractice] label id='{}' text='{}'",
            node->getID(), s ? s : "(null)"
        );
    });
    log::info("[GhostPractice] --- label dump end ---");
}

} // namespace ghost

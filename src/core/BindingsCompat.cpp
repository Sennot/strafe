#include "core/BindingsCompat.hpp"
#include "core/ModConfig.hpp"

namespace ghost::compat {

void walkTree(CCNode* root, const std::function<void(CCNode*)>& fn) {
    if (root == nullptr) return;
    fn(root);
    // Children may mutate during the walk only in init-time code; we never
    // mutate structure here, so a plain index walk is safe.
    if (auto* children = root->getChildren()) {
        for (auto* child : CCArrayExt<CCNode*>(children)) {
            walkTree(child, fn);
        }
    }
}

CCLabelProtocol* findLabelByPrefix(CCNode* root, const std::string& prefix) {
    CCLabelProtocol* found = nullptr;
    walkTree(root, [&](CCNode* node) {
        if (found != nullptr) return;
        // RTTI is enabled in Geode builds, sidecast to the label interface.
        auto* lp = dynamic_cast<CCLabelProtocol*>(node);
        if (lp == nullptr) return;
        const char* s = lp->getString();
        if (s == nullptr) return;
        if (startsWithCI(std::string(s), prefix)) found = lp;
    });
    return found;
}

} // namespace ghost::compat

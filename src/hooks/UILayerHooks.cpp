/**
 * UILayer hook — hides the practice checkpoint buttons when a disguised
 * session is running (also re-applied from togglePracticeMode next-frame,
 * because buttons may be created after UILayer::init).
 */
#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>

#include "core/ModConfig.hpp"
#include "core/SessionState.hpp"
#include "ui/PracticeUICleaner.hpp"

using namespace geode::prelude;
using namespace ghost;

class $modify(GPUILayer, UILayer) {
    bool init() {
        if (!UILayer::init()) return false;

        if (SessionState::get().active() && cfgBool("strip-practice-ui")) {
            PracticeUICleaner::get().onUILayer(this);
        }
        if (cfgBool("debug-dump-tree")) {
            PracticeUICleaner::get().dumpTree(this, "UILayer::init");
        }
        return true;
    }
};

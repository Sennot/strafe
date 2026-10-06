/**
 * PauseLayer hook — while a disguised session is active the pause menu must
 * not say "Practice Mode".
 */
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>

#include "core/ModConfig.hpp"
#include "core/SessionState.hpp"
#include "ui/PracticeUICleaner.hpp"

using namespace geode::prelude;
using namespace ghost;

class $modify(GPPauseLayer, PauseLayer) {
    bool init() {
        if (!PauseLayer::init()) return false;

        if (SessionState::get().active() && cfgBool("strip-practice-ui")) {
            PracticeUICleaner::get().fixPauseLayer(this);
        }
        if (cfgBool("debug-dump-tree")) {
            PracticeUICleaner::get().dumpTree(this, "PauseLayer");
        }
        return true;
    }
};

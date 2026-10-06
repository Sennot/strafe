/**
 * LevelCompleteLayer hook — the win screen shows the NORMAL-mode variant.
 *
 * LevelCompleteLayer::init(GJGameLevel*, bool, bool): one of the bools is
 * the practice flag on 2.2081. VERIFY(runtime): complete one level in
 * practice with "disguise-end-screen" ON and one with it OFF; if the
 * disguised screen is not the normal variant, adjust which bools are
 * forced false here (log line below prints the incoming values).
 *
 * NOTE: this changes PRESENTATION only. Stats/progress recording stays on
 * the engine's practice path on purpose — Ghost Practice never writes fake
 * completions to the save file (Volatile-Only principle, ARCHITECTURE.md).
 */
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelCompleteLayer.hpp>

#include "core/Events.hpp"
#include "core/ModConfig.hpp"
#include "core/SessionState.hpp"
#include "labels/LabelFreezer.hpp"
#include "state/CheckpointBank.hpp"
#include "ui/PracticeUICleaner.hpp"

using namespace geode::prelude;
using namespace ghost;

class $modify(GPLevelCompleteLayer, LevelCompleteLayer) {
    bool init(GJGameLevel* level, bool p1, bool p2) {
        log::info("[GhostPractice] LevelCompleteLayer::init bools: ({}, {})", p1, p2);

        if (SessionState::get().active() && cfgBool("disguise-end-screen")) {
            SessionState::get().end();
            LabelFreezer::get().unfreeze();
            CheckpointBank::get().clear();
            Events::get().sessionEnded.emit();
            return LevelCompleteLayer::init(level, false, false);
        }
        return LevelCompleteLayer::init(level, p1, p2);
    }
};

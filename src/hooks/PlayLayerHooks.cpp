/**
 * PlayLayer hooks — the orchestrator. Every other module reacts through
 * SessionState / Events; only this file talks to the engine directly.
 *
 * VERIFY list for GD 2.2081 / Geode 5.10 (see docs/ROADMAP.md phase 0):
 *   - PlayLayer::togglePracticeMode(bool) exists & fires on pause-menu
 *     practice toggle AND on startpos launches;
 *   - PlayLayer::destroyPlayer() is the death entry (else use resetLevel-pre);
 *   - PlayLayer::removeAllCheckpoints() exists;
 *   - m_level->m_normalPercent is an int field (progress-bar spoof write).
 */
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "core/BindingsCompat.hpp"
#include "core/Events.hpp"
#include "core/ModConfig.hpp"
#include "core/SessionState.hpp"
#include "labels/LabelFreezer.hpp"
#include "state/CheckpointBank.hpp"
#include "ui/PracticeUICleaner.hpp"
#include "ui/RestartController.hpp"

using namespace geode::prelude;
using namespace ghost;

namespace {

void endGhostSession() {
    if (!SessionState::get().active()) return;
    SessionState::get().end();
    LabelFreezer::get().unfreeze();
    CheckpointBank::get().clear();
    PracticeUICleaner::get().restoreAll();
    Events::get().labelsUnfreeze.emit();
    Events::get().sessionEnded.emit();
}

} // namespace

class $modify(GPPlayLayer, PlayLayer) {
    // Session entry point: practice ON (pause menu, or a startpos launch).
    void togglePracticeMode(bool practice) {
        PlayLayer::togglePracticeMode(practice);

        if (practice) {
            SessionState::get().begin(compat::currentPercent(this));
            Events::get().sessionStarted.emit(SessionState::get().baseline());
            // The practice buttons may be created right now or on the next
            // frame depending on where the toggle came from.
            Loader::get()->queueInMainThread([this]() {
                if (!SessionState::get().active()) return;
                if (cfgBool("strip-practice-ui")) {
                    PracticeUICleaner::get().onUILayer(m_uiLayer);
                }
                if (cfgBool("debug-dump-tree") && m_uiLayer) {
                    PracticeUICleaner::get().dumpTree(m_uiLayer, "UILayer(post-toggle)");
                }
            });
        } else {
            endGhostSession();
        }
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!SessionState::get().active()) return;

        SessionState::get().onTick(compat::currentPercent(this));

        // Progress-bar green "best" marker: in-memory spoof of the normal
        // percent. VOLATILE BY DESIGN — never persisted (ARCHITECTURE.md).
        if (m_level != nullptr) {
            m_level->m_normalPercent = static_cast<int>(
                SessionState::get().spoofedNormalPercent()
            );
        }

        // Pin the attempt label to "Attempt 1".
        if (cfgBool("spoof-attempt-label") && m_uiLayer != nullptr) {
            if (auto* label = compat::findLabelByPrefix(m_uiLayer, "Attempt")) {
                label->setString("Attempt 1");
            }
        }

        LabelFreezer::get().onFrame(this);
        RestartController::get().update(this);
    }

    void createCheckpoint() {
        // Node census before, so the cleaner can diff out the diamond.
        PracticeUICleaner::get().beforeCreateCheckpoint(this);
        PlayLayer::createCheckpoint();
        if (SessionState::get().active() && cfgBool("strip-practice-ui")) {
            PracticeUICleaner::get().afterCreateCheckpoint(this);
        }

        if (!SessionState::get().active()) return;

        float pct = compat::currentPercent(this);
        auto& banked = CheckpointBank::get().push();
        banked.extended.percent = pct;
        // banked.vanilla: P3 — Ref the CheckpointObject created just now
        // (VERIFY: last element of m_checkpointArray) so extended restore
        // has a partner object.

        SessionState::get().onCheckpoint(pct);
        Events::get().checkpointPlaced.emit(pct);

        if (cfgBool("freeze-labels")) {
            LabelFreezer::get().freeze(this);
            Events::get().labelsFreeze.emit(this);
        }
    }

    void removeCheckpoint() {
        PlayLayer::removeCheckpoint();
        if (!SessionState::get().active()) return;
        CheckpointBank::get().pop();
        // Un-bank to the PREVIOUS checkpoint if any (else baseline).
        float prev = CheckpointBank::get().previousPercent();
        if (prev >= 0.f) {
            SessionState::get().onCheckpoint(prev);
        } else {
            SessionState::get().onCheckpointRemoved();
        }
        Events::get().checkpointRemoved.emit();
    }

    void destroyPlayer() {  // VERIFY: death entry on 2.2081
        if (SessionState::get().active()) {
            float pct = compat::currentPercent(this);
            SessionState::get().onDeath(pct);
            Events::get().playerDied.emit(pct);
        }
        PlayLayer::destroyPlayer();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        if (SessionState::get().active()) {
            float from = compat::currentPercent(this);
            SessionState::get().onRestored(from);
            Events::get().restored.emit(from);
        }
    }

    void onQuit() {
        PlayLayer::onQuit();
        endGhostSession();
    }
};

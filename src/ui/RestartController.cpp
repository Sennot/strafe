#include "ui/RestartController.hpp"
#include "core/BindingsCompat.hpp"
#include "core/Events.hpp"
#include "core/ModConfig.hpp"
#include "core/SessionState.hpp"
#include "labels/LabelFreezer.hpp"
#include "state/CheckpointBank.hpp"

#ifdef GEODE_IS_WINDOWS
    #include <Windows.h>
#endif

namespace ghost {

RestartController& RestartController::get() {
    static RestartController instance;
    return instance;
}

void RestartController::update(PlayLayer* pl) {
    if (pl == nullptr || !SessionState::get().active()) return;
    if (!cfgBool("final-restart-hotkey")) return;

#ifdef GEODE_IS_WINDOWS
    const bool down = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;
    const bool pressed = down && !m_keyWasDown;
    m_keyWasDown = down;
    if (pressed) performFinalRestart(pl);
#else
    // Android port (roadmap): a floating touch button instead of a hotkey.
#endif
}

void RestartController::performFinalRestart(PlayLayer* pl) {
    if (pl == nullptr) return;

    log::info("[GhostPractice] final restart triggered");

    // 1) Our bookkeeping first.
    SessionState::get().onFinalRestart();
    LabelFreezer::get().unfreeze();
    CheckpointBank::get().clear();
    Events::get().finalRestart.emit();
    Events::get().labelsUnfreeze.emit();

    // 2) Engine: drop every checkpoint so respawn = from 0%.
    compat::removeAllCheckpointsCompat(pl);

    // 3) Restart the attempt.
    pl->resetLevel();
}

} // namespace ghost

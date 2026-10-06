#pragma once
/**
 * ghost::RestartController — the "final death" feature.
 *
 * Player story: "I'd rather wipe this run at 59% than continue from the
 * checkpoint" -> press F9 -> all checkpoints gone, run restarts from 0%,
 * labels unfreeze, Best Run resets. To a viewer this reads as starting a
 * fresh attempt; the engine quietly stays in practice mode.
 *
 * Input layer (temporary, P0): Win32 GetAsyncKeyState polling inside
 * PlayLayer::update. UPGRADE PATH (P2): Geode's native keybinds API
 * (BindManager, present since Geode 4) so the key is rebindable and
 * Android can get a touch button. Swap update() internals only — the
 * performFinalRestart() contract stays.
 */
#include <Geode/Geode.hpp>

namespace ghost {
using namespace geode::prelude;

class RestartController {
public:
    static RestartController& get();

    /// Polls the hotkey (call every frame from PlayLayer::update post-hook).
    void update(PlayLayer* pl);

    /// The actual action. Safe to call directly (e.g. from a future button).
    void performFinalRestart(PlayLayer* pl);

private:
    bool m_keyWasDown = false;
};

} // namespace ghost

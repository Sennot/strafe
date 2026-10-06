/**
 * Ghost Practice — entry point.
 *
 * Research mod for GD 2.2081 (Geode): practice sessions presented as
 * normal-mode runs. Presentation-only fakery; the save file is never
 * touched (Volatile-Only principle — see ARCHITECTURE.md).
 *
 * Hook ownership map (which file hooks what):
 *   hooks/PlayLayerHooks.cpp        — session lifecycle, checkpoints, freeze
 *   hooks/UILayerHooks.cpp          — practice buttons
 *   hooks/PauseLayerHooks.cpp       — pause menu title
 *   hooks/LevelCompleteHooks.cpp    — win screen variant
 */
#include <Geode/Geode.hpp>

#include "core/SessionState.hpp"
#include "mh/MegahackInterop.hpp"

using namespace geode::prelude;

$execute {
    log::info("[GhostPractice] loaded — Ghost Practice v{}", Mod::get()->getVersion().toString());

    if (auto* mhmod = Loader::get()->getLoadedMod("absolllute.megahack")) {
        log::info(
            "[GhostPractice] MegaHack '{}' detected; runtime interop will be attempted",
            mhmod->getVersion().toString()
        );
        // Resolve the MH extensions API now so the first checkpoint does not
        // pay the lookup cost (and we learn immediately if it failed).
        mh::loaded();
    } else {
        log::info("[GhostPractice] MegaHack not loaded; label freezing still applies "
                  "to any Cocos-rendered overlay labels (e.g. other HUD mods).");
    }
}

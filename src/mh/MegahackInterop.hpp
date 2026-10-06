#pragma once
/**
 * ghost::mh — runtime interop with Mega Hack v9 (absolllute.megahack).
 *
 * HOW (and why) this is safe:
 *   MH v9 exports an extensions API in namespace hackpro::extensions for
 *   exactly this purpose (other mods interoperating with it). We resolve the
 *   exports by their mangled names with GetProcAddress at runtime. No static
 *   linking, no binary patching, no offset hardcoding: if MH is absent, an
 *   export is renamed, or the whole thing crashes at resolve time, every
 *   function here degrades to a no-op / returns its fallback.
 *
 * Evidence for the export set (extracted from absolllute.megahack.geode
 * v9.1.3, 2026-10-07, see docs/FEASIBILITY.md):
 *   ?isHackEnabled@extensions@hackpro@@YA_NAEBV?$basic_string@...@Z
 *   ?setHackEnabled@extensions@hackpro@@YAXAEBV..._N@Z
 *   ??$getConfigVar@_N@extensions@hackpro@@YA_NAEBV...@Z   (bool)
 *   ??$setConfigVar@_N@extensions@hackpro@@YAXAEBV..._N@Z  (bool)
 *   ?hasCheatedInAttempt@safe_mode@utils@mega@@YA_NXZ
 *
 * NOTE: calling these crosses a DLL boundary with std::string arguments.
 * Both MH and Geode mods are built with MSVC against MSVCP140, so the STL
 * ABI matches in practice. If a future MH build changes compilers, the
 * resolve simply fails harmlessly (or the calls vanish behind `loaded()`).
 */
#include <string>

namespace ghost::mh {

bool loaded();

/// "v9.1.3" if resolvable via Geode's Loader, "" otherwise.
std::string version();

bool isHackEnabled(const std::string& hackName, bool fallback = false);

/// Returns false if MH is not loaded / export missing (not the hack state!).
bool setHackEnabled(const std::string& hackName, bool on);

bool getConfigBool(const std::string& key, bool fallback = false);
bool setConfigBool(const std::string& key, bool value);

/// MH safe-mode: true if the current attempt has cheats active.
bool hasCheatedInAttempt();

} // namespace ghost::mh

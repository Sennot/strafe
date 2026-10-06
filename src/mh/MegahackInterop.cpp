#include "mh/MegahackInterop.hpp"

#include <Geode/Geode.hpp>

#ifdef GEODE_IS_WINDOWS
    #include <Windows.h>
#endif

using namespace geode::prelude;

namespace ghost::mh {

#ifdef GEODE_IS_WINDOWS

// Mangled export names from absolllute.megahack v9.1.3 (win x64).
static constexpr const char* kExportIsHackEnabled =
    "?isHackEnabled@extensions@hackpro@@YA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
static constexpr const char* kExportSetHackEnabled =
    "?setHackEnabled@extensions@hackpro@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@_N@Z";
static constexpr const char* kExportGetConfigBool =
    "??$getConfigVar@_N@extensions@hackpro@@YA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";
static constexpr const char* kExportSetConfigBool =
    "??$setConfigVar@_N@extensions@hackpro@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@_N@Z";
static constexpr const char* kExportHasCheated =
    "?hasCheatedInAttempt@safe_mode@utils@mega@@YA_NXZ";

using IsHackEnabledFn   = bool(__cdecl*)(const std::string&);
using SetHackEnabledFn  = void(__cdecl*)(const std::string&, bool);
using GetConfigBoolFn   = bool(__cdecl*)(const std::string&);
using SetConfigBoolFn   = void(__cdecl*)(const std::string&, bool);
using HasCheatedFn      = bool(__cdecl*)();

struct Resolved {
    HMODULE module = nullptr;
    IsHackEnabledFn  isHackEnabled = nullptr;
    SetHackEnabledFn setHackEnabled = nullptr;
    GetConfigBoolFn  getConfigBool = nullptr;
    SetConfigBoolFn  setConfigBool = nullptr;
    HasCheatedFn     hasCheated = nullptr;
    bool tried = false;
};

static Resolved& resolve() {
    static Resolved r;
    if (r.tried) return r;
    r.tried = true;

    // A .geode package IS a PE dll; Geode loads it under its mod id name.
    // Try the common names; extend here if this ever misses.
    r.module = GetModuleHandleA("absolllute.megahack");
    if (r.module == nullptr) r.module = GetModuleHandleA("absolllute.megahack.dll");
    if (r.module == nullptr) r.module = GetModuleHandleA("absolllute.megahack.geode");
    if (r.module == nullptr) return r;

    r.isHackEnabled  = reinterpret_cast<IsHackEnabledFn>(GetProcAddress(r.module, kExportIsHackEnabled));
    r.setHackEnabled = reinterpret_cast<SetHackEnabledFn>(GetProcAddress(r.module, kExportSetHackEnabled));
    r.getConfigBool  = reinterpret_cast<GetConfigBoolFn>(GetProcAddress(r.module, kExportGetConfigBool));
    r.setConfigBool  = reinterpret_cast<SetConfigBoolFn>(GetProcAddress(r.module, kExportSetConfigBool));
    r.hasCheated     = reinterpret_cast<HasCheatedFn>(GetProcAddress(r.module, kExportHasCheated));

    log::info(
        "[GhostPractice] MegaHack module resolved @ {} (exports: isHackEnabled={} setHackEnabled={} getConfig={} setConfig={} hasCheated={})",
        static_cast<void*>(r.module),
        r.isHackEnabled != nullptr, r.setHackEnabled != nullptr,
        r.getConfigBool != nullptr, r.setConfigBool != nullptr,
        r.hasCheated != nullptr
    );
    return r;
}

bool loaded() {
    return resolve().module != nullptr;
}

std::string version() {
    if (auto* mod = Loader::get()->getLoadedMod("absolllute.megahack")) {
        return mod->getVersion().toString();
    }
    return "";
}

bool isHackEnabled(const std::string& hackName, bool fallback) {
    auto& r = resolve();
    if (r.isHackEnabled == nullptr) return fallback;
    return r.isHackEnabled(hackName);
}

bool setHackEnabled(const std::string& hackName, bool on) {
    auto& r = resolve();
    if (r.setHackEnabled == nullptr) return false;
    r.setHackEnabled(hackName, on);
    return true;
}

bool getConfigBool(const std::string& key, bool fallback) {
    auto& r = resolve();
    if (r.getConfigBool == nullptr) return fallback;
    return r.getConfigBool(key);
}

bool setConfigBool(const std::string& key, bool value) {
    auto& r = resolve();
    if (r.setConfigBool == nullptr) return false;
    r.setConfigBool(key, value);
    return true;
}

bool hasCheatedInAttempt() {
    auto& r = resolve();
    if (r.hasCheated == nullptr) return false;
    return r.hasCheated();
}

#else // non-Windows: stubs (Android port is a roadmap item)

bool loaded() { return false; }
std::string version() { return ""; }
bool isHackEnabled(const std::string&, bool fallback) { return fallback; }
bool setHackEnabled(const std::string&, bool) { return false; }
bool getConfigBool(const std::string&, bool fallback) { return fallback; }
bool setConfigBool(const std::string&, bool) { return false; }
bool hasCheatedInAttempt() { return false; }

#endif

} // namespace ghost::mh

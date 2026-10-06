#pragma once
/**
 * Thin wrapper around Geode settings access so the setting keys live in exactly
 * one place and modules never call Mod::get()->getSettingValue directly.
 */
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

namespace ghost {
using namespace geode::prelude;

inline bool cfgBool(const char* key) {
    return Mod::get()->getSettingValue<bool>(key);
}

inline std::string cfgString(const char* key) {
    return Mod::get()->getSettingValue<std::string>(key);
}

/// Split "a, b ,c" into {"a","b","c"} (trims spaces, drops empties).
inline std::vector<std::string> cfgCsv(const char* key) {
    std::vector<std::string> out;
    std::string s = cfgString(key);
    std::string cur;
    for (char c : s) {
        if (c == ',') {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        } else if (c != ' ' && c != '\t') {
            cur += c;
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

/// ASCII case-insensitive prefix match.
inline bool startsWithCI(const std::string& text, const std::string& prefix) {
    if (text.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        char a = text[i], b = prefix[i];
        if (a >= 'A' && a <= 'Z') a += 32;
        if (b >= 'A' && b <= 'Z') b += 32;
        if (a != b) return false;
    }
    return true;
}
} // namespace ghost

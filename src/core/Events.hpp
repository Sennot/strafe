#pragma once
/**
 * ghost::Events — tiny signal/slot bus that decouples modules from each other.
 *
 * Everything is emitted and consumed on the main (cocos) thread, inside
 * PlayLayer hooks. No locking needed.
 *
 * Adding a new reaction to something = Events::get().something.connect(...)
 * in your module's setup. Do NOT call other modules directly from hooks.
 */
#include <Geode/Geode.hpp>
#include <functional>
#include <vector>

namespace ghost {
using namespace geode::prelude;

template <typename... Args>
class Signal {
public:
    void connect(std::function<void(Args...)> fn) {
        m_slots.push_back(std::move(fn));
    }
    void emit(Args... args) {
        // copy: a slot may connect/disconnect during iteration
        auto slots = m_slots;
        for (auto& fn : slots) fn(args...);
    }

private:
    std::vector<std::function<void(Args...)>> m_slots;
};

struct Events {
    /// A disguised practice session started (practice toggled ON).
    Signal<float> sessionStarted;   // arg: baseline percent (0 or startpos %)
    /// Player placed a checkpoint (manual or auto).
    Signal<float> checkpointPlaced; // arg: percent at placement
    /// Player removed the last checkpoint.
    Signal<> checkpointRemoved;
    /// Player died (before practice respawn happens).
    Signal<float> playerDied;       // arg: percent at death
    /// Practice respawn finished; world restored from a checkpoint.
    Signal<float> restored;         // arg: percent restored from
    /// Final restart (F9): everything wiped, run restarts from 0.
    Signal<> finalRestart;
    /// Session ended (practice off, level quit, level completed).
    Signal<> sessionEnded;

    /// Freeze all matching overlay labels now (arg: subtree root to scan).
    Signal<CCNode*> labelsFreeze;
    /// Unfreeze labels and let them update again.
    Signal<> labelsUnfreeze;

    static Events& get() {
        static Events instance;
        return instance;
    }
};
} // namespace ghost

#pragma once
/**
 * ghost::SessionState — the single source of truth for a disguised session.
 *
 * One session = one stretch of play where the engine is in practice mode and
 * Ghost Practice is presenting it as a normal run. The session begins when
 * practice mode is toggled ON (from the pause menu, a startpos launch, etc.)
 * and ends when practice is toggled OFF, the level is quit, or the level is
 * completed.
 *
 * Semantics implemented here (user spec, 2026-10):
 *   - "Best Run" shown in real time: rises with live progress,
 *     but an un-banked death snaps it back to the last banked checkpoint.
 *   - Checkpoint at 25% banks 25%. Dying at 40% (no new checkpoint) =>
 *     Best Run falls back to 25. Reaching 50% and checkpointing banks 50.
 *   - Attempt label is pinned to "Attempt 1" for the whole session.
 *   - Final restart (F9) wipes the bank, restarts from 0%.
 */
#include <Geode/Geode.hpp>
#include "progress/BestRunPolicy.hpp"

namespace ghost {
using namespace geode::prelude;

class SessionState {
public:
    static SessionState& get();

    /// Called on practice ON. baselinePercent = percent where practice began
    /// (0 for a fresh level, >0 when starting from a startpos).
    void begin(float baselinePercent);
    /// Called on practice OFF / level quit / completion. Clears everything.
    void end();

    void onTick(float currentPercent);   ///< every frame while playing
    void onCheckpoint(float percent);    ///< checkpoint placed -> bank it
    void onCheckpointRemoved();          ///< last checkpoint deleted
    void onDeath(float percent);         ///< died before respawn
    void onRestored(float fromPercent);  ///< practice respawn finished
    void onFinalRestart();               ///< F9 wipe

    bool active() const { return m_active; }
    float baseline() const { return m_baselinePercent; }
    float banked() const { return m_bankedPercent; }
    int checkpointCount() const { return m_checkpointCount; }

    /// The Best Run value to display right now (already policy-adjusted).
    float bestRun() const;
    /// The attempt number to display ("Attempt N").
    int shownAttempt() const { return 1; }
    /// Value to spoof into the in-memory normal-mode percent (progress bar
    /// green segment). Volatile only — never saved (see ARCHITECTURE.md).
    float spoofedNormalPercent() const;

    /// Swap policy at runtime (BankingPolicy default, StartposPolicy P4).
    void setPolicy(BestRunPolicy* policy);

private:
    SessionState() = default;

    bool m_active = false;
    float m_baselinePercent = 0.f;
    float m_bankedPercent = 0.f;      // last CONFIRMED (checkpointed) progress
    float m_liveMaxPercent = 0.f;     // max reached since last bank/death
    bool m_diedUnbanked = false;      // death after bank, before next bank
    int m_checkpointCount = 0;

    BestRunPolicy* m_policy = &BankingPolicy::instance();
};

} // namespace ghost
